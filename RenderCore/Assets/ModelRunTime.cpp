// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "ModelRunTime.h"
#include "ModelRunTimeInternal.h"
#include "DelayedDrawCall.h"
#include "MaterialScaffold.h"
#include "TransformationCommands.h"
#include "AssetUtils.h"     // maybe only needed for chunk ids
#include "Material.h"
#include "RawAnimationCurve.h"
#include "SharedStateSet.h"
#include "DeferredShaderResource.h"

#include "../Techniques/Techniques.h"
#include "../Techniques/ResourceBox.h"
#include "../Techniques/ParsingContext.h"
#include "../Techniques/CommonResources.h"
#include "../Techniques/PredefinedCBLayout.h"

#include "../Metal/Buffer.h"
#include "../Metal/State.h"
#include "../Metal/DeviceContext.h"
#include "../Metal/DeviceContextImpl.h"
#include "../Resource.h"
#include "../RenderUtils.h"
#include "../../Assets/AssetUtils.h"

#include "../../Assets/BlockSerializer.h"
#include "../../Assets/ChunkFile.h"
#include "../../Assets/IntermediateAssets.h"

#include "../../Core/Exceptions.h"
#include "../../Utility/PtrUtils.h"
#include "../../Utility/Streams/FileUtils.h"
#include "../../Utility/IteratorUtils.h"
#include "../../Utility/StringFormat.h"
#include "../../Math/Transformations.h"
#include "../../ConsoleRig/Console.h"

#include <string>

#pragma warning(disable:4189)

namespace RenderCore { namespace Assets
{
    using ::Assets::ResChar;

    /// <summary>Internal namespace with utilities for constructing models</summary>
    /// These functions are normally used within the constructor of ModelRenderer
    namespace ModelConstruction
    {
        static const std::string DefaultShader = "illum";

        static size_t InsertOrCombine(std::vector<std::vector<uint8>>& dest, std::vector<uint8>&& compare)
        {
            assert(compare.size());
            for (auto i = dest.cbegin(); i!=dest.cend(); ++i) {
                if (i->size() == compare.size() && (XlCompareMemory(AsPointer(i->begin()), AsPointer(compare.begin()), i->size()) == 0)) {
                    return std::distance(dest.cbegin(), i);
                }
            }

            dest.push_back(std::forward<std::vector<uint8>>(compare));
            return dest.size()-1;
        }

        struct SubMatResources
        { 
            unsigned _shaderName; 
            unsigned _matParams; 
            unsigned _constantBuffer; 
            unsigned _texturesIndex; 
            unsigned _renderStateSet;
            DelayStep _delayStep;
        };

        static const ModelCommandStream::GeoCall& GetGeoCall(const ModelScaffold& scaffold, unsigned geoCallIndex)
        {
                //  get the "RawGeometry" object in the given scaffold for the give
                //  geocall index. This will query both unskinned and skinned raw calls
            auto& cmdStream = scaffold.CommandStream();
            auto geoCallCount = cmdStream.GetGeoCallCount();

            return (geoCallIndex < geoCallCount) ? cmdStream.GetGeoCall(geoCallIndex) : cmdStream.GetSkinCall(geoCallIndex - geoCallCount);
        }
        
        static const RawGeometry& GetGeo(const ModelScaffold& scaffold, unsigned geoCallIndex)
        {
                //  get the "RawGeometry" object in the given scaffold for the give
                //  geocall index. This will query both unskinned and skinned raw calls
            auto& meshData = scaffold.ImmutableData();
            auto geoCallCount = scaffold.CommandStream().GetGeoCallCount();

            auto& geoCall = GetGeoCall(scaffold, geoCallIndex);
            return (geoCallIndex < geoCallCount) ? meshData._geos[geoCall._geoId] : (RawGeometry&)meshData._boundSkinnedControllers[geoCall._geoId];
        }

        static unsigned GetDrawCallCount(const ModelScaffold& scaffold, unsigned geoCallIndex)
        {
            return (unsigned)GetGeo(scaffold, geoCallIndex)._drawCallsCount;
        }

        static MaterialGuid ScaffoldMaterialIndex(const ModelScaffold& scaffold, unsigned geoCallIndex, unsigned drawCallIndex)
        {
            auto& meshData = scaffold.ImmutableData();
            auto geoCallCount = scaffold.CommandStream().GetGeoCallCount();

            auto& geoCall = GetGeoCall(scaffold, geoCallIndex);
            auto& geo = (geoCallIndex < geoCallCount) ? meshData._geos[geoCall._geoId] : (RawGeometry&)meshData._boundSkinnedControllers[geoCall._geoId];
            unsigned subMatI = geo._drawCalls[drawCallIndex]._subMaterialIndex;

                //  the "sub material index" in the draw call is an index
                //  into the array in the geo call
            if (subMatI < geoCall._materialCount) {
                return geoCall._materialGuids[subMatI];
            }

            return ~unsigned(0x0);
        }

        template <typename T> static bool AtLeastOneValidDrawCall(const RawGeometry& geo, const ModelScaffold& scaffold, unsigned geoCallIndex, std::vector<std::pair<MaterialGuid, T>>& subMatResources)
        {
                //  look for at least one valid draw call in this geo instance
                //  a valid draw call should have got shader and material information bound
            bool atLeastOneValidDrawCall = false;
            for (unsigned di=0; di<geo._drawCallsCount; ++di) {
                auto& d = geo._drawCalls[di];
                if (d._indexCount) {
                    auto subMatIndex = ScaffoldMaterialIndex(scaffold, geoCallIndex, di);
                    auto snm = LowerBound(subMatResources, subMatIndex);
                    if (snm != subMatResources.cend() && snm->first == subMatIndex) {
                        return true;
                    }
                }
            }

            return false;       // didn't find any draw calls with good material information. This whole geo object can be ignored.
        }

        static void LoadBlock(BasicFile& file, uint8 destination[], size_t fileOffset, size_t readSize)
        {
            file.Seek(fileOffset, SEEK_SET);
            file.Read(destination, 1, readSize);
        }

        static bool HasElement(const GeoInputAssembly& ia, const char name[])
        {
            auto end = &ia._elements[ia._elementCount];
            return std::find_if(
                ia._elements, end, 
                [=](const VertexElement& ele) { return !XlCompareString(ele._semantic, name); }) != end;
        }

        #if defined(_DEBUG)
            static std::string MakeDescription(const ParameterBox& paramBox)
            {
                std::vector<std::pair<const utf8*, std::string>> defines;
                BuildStringTable(defines, paramBox);
                std::stringstream dst;
                for (auto i=defines.cbegin(); i!=defines.cend(); ++i) {
                    if (i != defines.cbegin()) { dst << "; "; }
                    dst << i->first << " = " << i->second;
                }
                return dst.str();
            }
            
            class ParamBoxDescriptions
            {
            public:
                void Add(unsigned index, const ParameterBox& box)
                {
                    auto existing = LowerBound(_descriptions, index);
                    if (existing == _descriptions.end() || existing->first != index) {
                        _descriptions.insert(existing, std::make_pair(index, MakeDescription(box)));
                    }
                }
                std::vector<std::pair<unsigned,std::string>> _descriptions;
            };
        #else
            class ParamBoxDescriptions
            {
            public:
                void Add(unsigned index, const ParameterBox& box) {}
            };
        #endif

        static unsigned BuildGeoParamBox(
            const GeoInputAssembly& ia, SharedStateSet& sharedStateSet, 
            ModelConstruction::ParamBoxDescriptions& paramBoxDesc, bool normalFromSkinning)
        {
                //  Build a parameter box for this geometry configuration. The input assembly
            ParameterBox geoParameters;
            if (HasElement(ia, "TEXCOORD"))     { geoParameters.SetParameter((const utf8*)"GEO_HAS_TEXCOORD", 1); }
            if (HasElement(ia, "COLOR"))        { geoParameters.SetParameter((const utf8*)"GEO_HAS_COLOUR", 1); }
            if (HasElement(ia, "NORMAL") || normalFromSkinning) 
                { geoParameters.SetParameter((const utf8*)"GEO_HAS_NORMAL", 1); }
            if (HasElement(ia, "TEXTANGENT"))      { geoParameters.SetParameter((const utf8*)"GEO_HAS_TANGENT_FRAME", 1); }
            if (HasElement(ia, "TEXBITANGENT"))    { geoParameters.SetParameter((const utf8*)"GEO_HAS_BITANGENT", 1); }
            if (HasElement(ia, "BONEINDICES") && HasElement(ia, "BONEWEIGHTS"))
                { geoParameters.SetParameter((const utf8*)"GEO_HAS_SKIN_WEIGHTS", 1); }
            auto result = sharedStateSet.InsertParameterBox(geoParameters);
            paramBoxDesc.Add(result, geoParameters);
            return result;
        }

        static const auto DefaultNormalsTextureBindingHash = ParameterBox::MakeParameterNameHash("NormalsTexture");

        static std::vector<std::pair<MaterialGuid, SubMatResources>> BuildMaterialResources(
            const ModelScaffold& scaffold, const MaterialScaffold& matScaffold,
            SharedStateSet& sharedStateSet, unsigned levelOfDetail,
            std::vector<uint64>& textureBindPoints,
            std::vector<std::vector<uint8>>& prescientMaterialConstantBuffers,
            ParamBoxDescriptions& paramBoxDesc, const Techniques::PredefinedCBLayout& cbLayout,
            const ::Assets::DirectorySearchRules* searchRules)
        {
            std::vector<std::pair<MaterialGuid, SubMatResources>> materialResources;

            auto& cmdStream = scaffold.CommandStream();
            auto& meshData = scaffold.ImmutableData();

            auto geoCallCount = cmdStream.GetGeoCallCount();
            auto skinCallCount = cmdStream.GetSkinCallCount();
            for (unsigned gi=0; gi<geoCallCount + skinCallCount; ++gi) {
                auto& geoInst = (gi < geoCallCount) ? cmdStream.GetGeoCall(gi) : cmdStream.GetSkinCall(gi - geoCallCount);
                if (geoInst._levelOfDetail != levelOfDetail) { continue; }

                    //  Lookup the mesh geometry and material information from their respective inputs.
                auto& geo = (gi < geoCallCount) ? meshData._geos[geoInst._geoId] : (RawGeometry&)meshData._boundSkinnedControllers[geoInst._geoId];
                for (unsigned di=0; di<geo._drawCallsCount; ++di) {
                    auto scaffoldMatIndex = ScaffoldMaterialIndex(scaffold, gi, di);
                    auto existing = LowerBound(materialResources, scaffoldMatIndex);
                    if (existing == materialResources.cend() || existing->first != scaffoldMatIndex) {
                        materialResources.insert(existing, std::make_pair(scaffoldMatIndex, SubMatResources()));
                    }
                }
            }

                // fill in the details for all of the material references we found
            for (auto i=materialResources.begin(); i!=materialResources.end(); ++i) {
                std::string shaderName = DefaultShader;
                i->second._shaderName = sharedStateSet.InsertShaderName(shaderName);
                i->second._texturesIndex = (unsigned)std::distance(materialResources.begin(), i);
            }

                // build material constants
            for (auto i=materialResources.begin(); i!=materialResources.end(); ++i) {
                auto* matData = matScaffold.GetMaterial(i->first);
                auto cbData = matData ? cbLayout.BuildCBDataAsVector(matData->_constants) : std::vector<uint8>(cbLayout._cbSize, uint8(0));

                i->second._constantBuffer = 
                    (unsigned)InsertOrCombine(
                        prescientMaterialConstantBuffers, 
                        std::move(cbData));
            }

                // configure the texture bind points array & material parameters box
            for (auto i=materialResources.begin(); i!=materialResources.end(); ++i) {
                ParameterBox materialParamBox;
                RenderStateSet stateSet;

                    //  we need to create a list of all of the texture bind points that are referenced
                    //  by all of the materials used here. They will end up in sorted order
                auto* matData = matScaffold.GetMaterial(i->first);
                if (matData) {
                    const auto& materialScaffoldData = i->first;
                
                    materialParamBox = matData->_matParams;
                    stateSet = matData->_stateSet;
                
                    for (auto param = matData->_bindings.Begin(); !param.IsEnd(); ++param) {
                        materialParamBox.SetParameter(
                            (const utf8*)(StringMeld<64, utf8>() << "RES_HAS_" << param.Name()), 1);
                
                        auto bindNameHash = Hash64((const char*)param.Name());
                        auto q = std::lower_bound(textureBindPoints.begin(), textureBindPoints.end(), bindNameHash);
                        if (q != textureBindPoints.end() && *q == bindNameHash) { continue; }
                        textureBindPoints.insert(q, bindNameHash);
                    }

                    auto boundNormalMapName = matData->_bindings.GetString<::Assets::ResChar>(DefaultNormalsTextureBindingHash);
                    if (!boundNormalMapName.empty()) {
                            //  We need to decide whether the normal map is "DXT" 
                            //  format or not. This information isn't in the material
                            //  itself; we actually need to look at the texture file
                            //  to see what format it is. Unfortunately that means
                            //  opening the texture file to read it's header. However
                            //  we can accelerate it a bit by caching the result
                        bool isDxtNormalMap = false;
                        if (searchRules) {
                            ::Assets::ResChar resolvedPath[MaxPath];
                            searchRules->ResolveFile(resolvedPath, dimof(resolvedPath), boundNormalMapName.c_str());
                            isDxtNormalMap = IsDXTNormalMap(resolvedPath);
                        } else 
                            isDxtNormalMap = IsDXTNormalMap(boundNormalMapName);
                        materialParamBox.SetParameter((const utf8*)"RES_HAS_NormalsTexture_DXT", isDxtNormalMap);
                    }
                }

                i->second._matParams = sharedStateSet.InsertParameterBox(materialParamBox);
                i->second._renderStateSet = sharedStateSet.InsertRenderStateSet(stateSet);

                i->second._delayStep = 
                    (stateSet._forwardBlendOp == Metal::BlendOp::NoBlending) ? DelayStep::OpaqueRender : DelayStep::PostDeferred;

                paramBoxDesc.Add(i->second._matParams, materialParamBox);
            }

            return materialResources;
        }

        std::vector<const DeferredShaderResource*> BuildBoundTextures(
            const ModelScaffold& scaffold, const MaterialScaffold& matScaffold,
            const ::Assets::DirectorySearchRules* searchRules,
            const std::vector<std::pair<MaterialGuid, SubMatResources>>& materialResources,
            const std::vector<uint64>& textureBindPoints, unsigned textureSetCount,
            std::vector<::Assets::rstring>& boundTextureNames)
        {
            auto texturesPerMaterial = textureBindPoints.size();

            std::vector<const DeferredShaderResource*> boundTextures;
            boundTextures.resize(textureSetCount * texturesPerMaterial, nullptr);
            DEBUG_ONLY(boundTextureNames.resize(textureSetCount * texturesPerMaterial));

            for (auto i=materialResources.begin(); i!=materialResources.end(); ++i) {
                unsigned textureSetIndex = i->second._texturesIndex;
        
                auto* matData = matScaffold.GetMaterial(i->first);
                if (!matData) { continue; }

                for (auto param=matData->_bindings.Begin(); !param.IsEnd(); ++param) {
                    auto bindNameHash = Hash64((const char*)param.Name());

                    auto i = std::find(textureBindPoints.cbegin(), textureBindPoints.cend(), bindNameHash);
                    assert(i!=textureBindPoints.cend() && *i == bindNameHash);
                    auto index = std::distance(textureBindPoints.cbegin(), i);

                    auto resourceName = matData->_bindings.GetString<::Assets::ResChar>(param.HashName());
                    if (resourceName.empty()) continue;
                
                    TRY {
                            // note --  Ideally we want to do this filename resolve in a background thread
                            //          however, it doesn't work well with our resources system. Because we're
                            //          expecting to create the DeferredShaderResource from a definitive file
                            //          name, something that can be matched against other (already loaded) resources.
                            //          So we need something different here... Something that can resolve a filename
                            //          in the background, and then return a shareable resource afterwards
                        auto dsti = textureSetIndex*texturesPerMaterial + index;
                        if (searchRules) {
                            ResChar resolvedPath[MaxPath];
                            searchRules->ResolveFile(resolvedPath, dimof(resolvedPath), resourceName.c_str());
                            boundTextures[dsti] = &::Assets::GetAssetDep<DeferredShaderResource>(resolvedPath);
                            DEBUG_ONLY(boundTextureNames[dsti] = resolvedPath);
                        } else {
                            boundTextures[dsti] = &::Assets::GetAssetDep<DeferredShaderResource>(resourceName.c_str());
                            DEBUG_ONLY(boundTextureNames[dsti] = resourceName);
                        }
                    } CATCH (const ::Assets::Exceptions::InvalidAsset&) {
                        LogWarning << "Warning -- shader resource (" << resourceName << ") couldn't be found";
                    } CATCH_END
                }
            }

            return std::move(boundTextures);
        }

        class BuffersUnderConstruction
        {
        public:
            unsigned _vbSize;
            unsigned _ibSize;

            unsigned AllocateIB(unsigned size, RenderCore::Metal::NativeFormat::Enum format)
            {
                unsigned allocation = _ibSize;

                    // we have to align the index buffer offset correctly
                unsigned indexStride = (format == RenderCore::Metal::NativeFormat::R32_UINT)?4:2;
                unsigned rem = _ibSize % indexStride;
                if (rem != 0) {
                    allocation += indexStride - rem;
                }

                _ibSize = allocation + size;
                return allocation;
            }

            unsigned AllocateVB(unsigned size)
            {
                unsigned result = _vbSize;
                _vbSize += size;
                return result;
            }

            BuffersUnderConstruction() : _vbSize(0), _ibSize(0) {}
        };
    }

    unsigned BuildLowLevelInputAssembly(
        Metal::InputElementDesc dst[], unsigned dstMaxCount,
        const VertexElement* source, unsigned sourceCount,
        unsigned lowLevelSlot)
    {
        unsigned vertexElementCount = 0;
        for (unsigned i=0; i<sourceCount; ++i) {
            auto& sourceElement = source[i];
            assert((vertexElementCount+1) <= dstMaxCount);
            if ((vertexElementCount+1) <= dstMaxCount) {
                    // in some cases we need multiple "slots". When we have multiple slots, the vertex data 
                    //  should be one after another in the vb (that is, not interleaved)
                dst[vertexElementCount++] = Metal::InputElementDesc(
                    sourceElement._semantic, sourceElement._semanticIndex,
                    Metal::NativeFormat::Enum(sourceElement._format), lowLevelSlot, sourceElement._startOffset);
            }
        }
        return vertexElementCount;
    }

    ModelRenderer::ModelRenderer(
        const ModelScaffold& scaffold, const MaterialScaffold& matScaffold,
        SharedStateSet& sharedStateSet, 
        const ::Assets::DirectorySearchRules* searchRules, unsigned levelOfDetail)
    {
        using namespace ModelConstruction;

        const auto& cbLayout = ::Assets::GetAssetDep<Techniques::PredefinedCBLayout>("game/xleres/BasicMaterialConstants.txt");

            // build the underlying objects required to render the given scaffold 
            //  (at the given level of detail)
        std::vector<uint64> textureBindPoints;
        std::vector<std::vector<uint8>> prescientMaterialConstantBuffers;
        ModelConstruction::ParamBoxDescriptions paramBoxDesc;
        auto materialResources = BuildMaterialResources(
            scaffold, matScaffold, sharedStateSet, levelOfDetail,
            textureBindPoints, prescientMaterialConstantBuffers,
            paramBoxDesc, cbLayout, searchRules);

            // one "textureset" for each sub material (though, in theory, we could 
            // combine texture sets for materials that share the same textures
        unsigned textureSetCount = unsigned(materialResources.size());

        auto& cmdStream = scaffold.CommandStream();
        auto& meshData = scaffold.ImmutableData();

            //  First we need to bind each draw call to a material in our
            //  material scaffold. Then we need to find the superset of all bound textures
            //      This superset will be used to initialize all of the technique 
            //      input interfaces
            //  The final resolved texture is defined by the "meshCall" object and by the 
            //  "drawCall" objects. MeshCall gives us the material id, and the draw call 
            //  gives us the sub material id.

            //  Note that we can have cases where the same mesh is referenced multiple times by 
            //  a single "geo call". In these cases, we want the mesh data to be stored once
            //  in the vertex buffer / index buffer but for there to be multiple sets of "draw calls"
            //  So, we have to separate the mesh processing from the draw call processing here
        
        BuffersUnderConstruction workingBuffers;
        auto geoCallCount = cmdStream.GetGeoCallCount();
        auto skinCallCount = cmdStream.GetSkinCallCount();
        auto drawCallCount = 0;
        for (unsigned c=0; c<geoCallCount + skinCallCount; ++c) { drawCallCount += GetDrawCallCount(scaffold, c); }

            //  We should calculate the size we'll need for nascentIB & nascentVB first, 
            //  so we don't have to do any reallocations

            ////////////////////////////////////////////////////////////////////////
                //          u n s k i n n e d   g e o                           //
        std::vector<Pimpl::Mesh> meshes;
        std::vector<Pimpl::MeshAndDrawCall> drawCalls;
        std::vector<Pimpl::DrawCallResources> drawCallRes;
        drawCalls.reserve(drawCallCount);
        drawCallRes.reserve(drawCallCount);

        for (unsigned gi=0; gi<geoCallCount; ++gi) {
            auto& geoInst = cmdStream.GetGeoCall(gi);
            if (geoInst._levelOfDetail != levelOfDetail) { continue; }

                //  Check to see if this mesh has at least one valid draw call. If there
                //  is none, we can skip it completely
            assert(geoInst._geoId < meshData._geoCount);
            auto& geo = meshData._geos[geoInst._geoId];
            if (!AtLeastOneValidDrawCall(geo, scaffold, gi, materialResources)) { continue; }

                // if we encounter the same mesh multiple times, we don't need to store it every time
            auto mesh = FindIf(meshes, [=](const Pimpl::Mesh& mesh) { return mesh._id == geoInst._geoId; });
            if (mesh == meshes.end()) {
                meshes.push_back(
                    Pimpl::BuildMesh(
                        geoInst, geo, workingBuffers, sharedStateSet,
                        AsPointer(textureBindPoints.cbegin()), (unsigned)textureBindPoints.size(),
                        paramBoxDesc));
                mesh = meshes.end()-1;
            }

                // setup the "Draw call" objects next
            for (unsigned di=0; di<geo._drawCallsCount; ++di) {
                auto& d = geo._drawCalls[di];
                if (!d._indexCount) { continue; }

                auto scaffoldMatIndex = ScaffoldMaterialIndex(scaffold, gi, di);
                auto matResI = LowerBound(materialResources, scaffoldMatIndex);
                if (matResI == materialResources.cend() || matResI->first != scaffoldMatIndex) {
                    continue;   // missing shader name means a "no-draw" shader
                }
                const auto& matRes = matResI->second;

                    //  "Draw call resources" are used when performing this draw call.
                    //  They help select the right shader, and are also required for
                    //  setting the graphics state (bound textures and shader constants, etc)
                    //  We can initialise them now using some information from the geometry
                    //  object and some information from the material.
                Pimpl::DrawCallResources res(
                    matRes._shaderName,
                    mesh->_geoParamBox, matRes._matParams, 
                    matRes._texturesIndex, matRes._constantBuffer,
                    matRes._renderStateSet, matRes._delayStep, scaffoldMatIndex);
                drawCallRes.push_back(res);
                drawCalls.push_back(std::make_pair(gi, d));
            }
        }

            ////////////////////////////////////////////////////////////////////////
                //          s k i n n e d   g e o                           //
        std::vector<Pimpl::SkinnedMesh> skinnedMeshes;
        std::vector<Pimpl::MeshAndDrawCall> skinnedDrawCalls;
        std::vector<Pimpl::SkinnedMeshAnimBinding> skinnedBindings;

        for (unsigned gi=0; gi<skinCallCount; ++gi) {
            auto& geoInst = cmdStream.GetSkinCall(gi);
            if (geoInst._levelOfDetail != levelOfDetail) { continue; }

                //  Check to see if this mesh has at least one valid draw call. If there
                //  is none, we can skip it completely
            assert(geoInst._geoId < meshData._boundSkinnedControllerCount);
            auto& geo = meshData._boundSkinnedControllers[geoInst._geoId];
            if (!AtLeastOneValidDrawCall(geo, scaffold, unsigned(geoCallCount + gi), materialResources)) { continue; }

                // if we encounter the same mesh multiple times, we don't need to store it every time
            auto mesh = FindIf(skinnedMeshes, [=](const Pimpl::SkinnedMesh& mesh) { return mesh._id == geoInst._geoId; });
            if (mesh == skinnedMeshes.end()) {
                skinnedMeshes.push_back(
                    Pimpl::BuildMesh(geoInst, geo, workingBuffers, sharedStateSet, 
                        AsPointer(textureBindPoints.cbegin()), (unsigned)textureBindPoints.size(),
                        paramBoxDesc));
                skinnedBindings.push_back(
                    Pimpl::BuildAnimBinding(
                        geoInst, geo, sharedStateSet, 
                        AsPointer(textureBindPoints.cbegin()), (unsigned)textureBindPoints.size()));

                mesh = skinnedMeshes.end()-1;
            }

            for (unsigned di=0; di<geo._drawCallsCount; ++di) {
                auto& d = geo._drawCalls[di];
                if (!d._indexCount) { continue; }
                
                auto scaffoldMatIndex = ScaffoldMaterialIndex(scaffold, unsigned(geoCallCount + gi), di);
                auto subMatResI = LowerBound(materialResources, scaffoldMatIndex);
                if (subMatResI == materialResources.cend() || subMatResI->first != scaffoldMatIndex) {
                    continue;   // missing shader name means a "no-draw" shader
                }

                auto& matRes = subMatResI->second;
                Pimpl::DrawCallResources res(
                    matRes._shaderName,
                    mesh->_geoParamBox, matRes._matParams, 
                    matRes._texturesIndex, matRes._constantBuffer,
                    matRes._renderStateSet, matRes._delayStep, scaffoldMatIndex);

                drawCallRes.push_back(res);
                skinnedDrawCalls.push_back(std::make_pair(gi, d));
            }
        }

            ////////////////////////////////////////////////////////////////////////
                //
                //  We have to load the "large blocks" from the file here
                //      -- todo -- this part can be pushed into the background
                //          using the buffer uploads system
                //
        std::vector<uint8> nascentVB, nascentIB;
        nascentVB.resize(workingBuffers._vbSize);
        nascentIB.resize(workingBuffers._ibSize);

        BasicFile file(scaffold.Filename().c_str(), "rb");
        auto largeBlocksOffset = scaffold.LargeBlocksOffset();

        for (auto m=meshes.begin(); m!=meshes.end(); ++m) {
            LoadBlock(file, &nascentIB[m->_ibOffset], largeBlocksOffset + m->_sourceFileIBOffset, m->_sourceFileIBSize);
            LoadBlock(file, &nascentVB[m->_vbOffset], largeBlocksOffset + m->_sourceFileVBOffset, m->_sourceFileVBSize);
        }

        for (auto m=skinnedMeshes.begin(); m!=skinnedMeshes.end(); ++m) {
            LoadBlock(file, &nascentIB[m->_ibOffset], largeBlocksOffset + m->_sourceFileIBOffset, m->_sourceFileIBSize);
            LoadBlock(file, &nascentVB[m->_vbOffset], largeBlocksOffset + m->_sourceFileVBOffset, m->_sourceFileVBSize);
            for (unsigned s=0; s<Pimpl::SkinnedMesh::VertexStreams::Max; ++s) {
                LoadBlock(file, &nascentVB[m->_extraVbOffset[s]], largeBlocksOffset + m->_sourceFileExtraVBOffset[s], m->_sourceFileExtraVBSize[s]);
            }
        }

            ////////////////////////////////////////////////////////////////////////
                //  now that we have a list of all of the sub materials used, and we know how large the resource 
                //  interface is, we build an array of deferred shader resources for shader inputs.
        std::vector<::Assets::rstring> boundTextureNames;
        auto boundTextures = BuildBoundTextures(
            scaffold, matScaffold, searchRules,
            materialResources, textureBindPoints, textureSetCount,
            boundTextureNames);

            ////////////////////////////////////////////////////////////////////////

        std::vector<Metal::ConstantBuffer> finalConstantBuffers;
        for (auto cb=prescientMaterialConstantBuffers.cbegin(); cb!=prescientMaterialConstantBuffers.end(); ++cb) {
            assert(cb->size());
            Metal::ConstantBuffer newCB(AsPointer(cb->begin()), cb->size());
            finalConstantBuffers.push_back(std::move(newCB));
        }

        Metal::VertexBuffer vb(AsPointer(nascentVB.begin()), nascentVB.size());
        Metal::IndexBuffer ib(AsPointer(nascentIB.begin()), nascentIB.size());

            ////////////////////////////////////////////////////////////////////////

        _validationCallback = std::make_shared<::Assets::DependencyValidation>();
        ::Assets::RegisterAssetDependency(_validationCallback, cbLayout.GetDependencyValidation());
        for (const auto& t:boundTextures) if (t) ::Assets::RegisterAssetDependency(_validationCallback, t->GetDependencyValidation());       // rebuild the entire renderer if any texture changes

        auto pimpl = std::make_unique<Pimpl>();

        pimpl->_vertexBuffer = std::move(vb);
        pimpl->_indexBuffer = std::move(ib);
        pimpl->_meshes = std::move(meshes);
        pimpl->_skinnedMeshes = std::move(skinnedMeshes);
        pimpl->_skinnedBindings = std::move(skinnedBindings);

        pimpl->_drawCalls = std::move(drawCalls);
        pimpl->_drawCallRes = std::move(drawCallRes);
        pimpl->_skinnedDrawCalls = std::move(skinnedDrawCalls);

        pimpl->_boundTextures = std::move(boundTextures);
        pimpl->_constantBuffers = std::move(finalConstantBuffers);
        pimpl->_texturesPerMaterial = textureBindPoints.size();

        pimpl->_scaffold = &scaffold;
        pimpl->_levelOfDetail = levelOfDetail;
        
        #if defined(_DEBUG)
            pimpl->_vbSize = (unsigned)nascentVB.size();
            pimpl->_ibSize = (unsigned)nascentIB.size();
            pimpl->_boundTextureNames = std::move(boundTextureNames);
            pimpl->_paramBoxDesc = std::move(paramBoxDesc._descriptions);
        #endif
        _pimpl = std::move(pimpl);

        DEBUG_ONLY(LogReport());
    }

    ModelRenderer::~ModelRenderer()
    {}

    

    class ModelRenderingBox
    {
    public:
        class Desc {};

        Metal::ConstantBuffer _localTransformBuffer;
        ModelRenderingBox(const Desc&)
        {
            Metal::ConstantBuffer localTransformBuffer(nullptr, sizeof(Techniques::LocalTransformConstants));
            _localTransformBuffer = std::move(localTransformBuffer);
        }
        ~ModelRenderingBox() {}
    };

    Metal::BoundUniforms*    ModelRenderer::Pimpl::BeginVariation(
            const ModelRendererContext& context,
            const SharedStateSet&   sharedStateSet,
            unsigned                drawCallIndex,
            TechniqueInterface      techniqueInterface) const
    {
        static Utility::ParameterBox tempGlobalStatesBox;

        const auto& res = _drawCallRes[drawCallIndex];
        sharedStateSet.BeginRenderState(context, tempGlobalStatesBox, res._renderStateSet);
        return sharedStateSet.BeginVariation(
            context, res._shaderName, techniqueInterface, res._geoParamBox, res._materialParamBox);
    }

    auto ModelRenderer::Pimpl::BeginGeoCall(
            const ModelRendererContext& context,
            Metal::ConstantBuffer&      localTransformBuffer,
            const MeshToModel*          transforms,
            const Float4x4&             modelToWorld,
            unsigned                    geoCallIndex) const -> TechniqueInterface
    {
        auto& cmdStream = _scaffold->CommandStream();
        auto& geoCall = cmdStream.GetGeoCall(geoCallIndex);

        if (transforms) {
            auto localToModel = transforms->GetMeshToModel(geoCall._transformMarker);
            Techniques::LocalTransformConstants trans;
            trans = Techniques::MakeLocalTransform(Combine(localToModel, modelToWorld), ExtractTranslation(context._parserContext->GetProjectionDesc()._cameraToWorld));
            localTransformBuffer.Update(*context._context, &trans, sizeof(trans));
        }

            // todo -- should be possible to avoid this search
        auto mesh = FindIf(_meshes, [=](const Pimpl::Mesh& mesh) { return mesh._id == geoCall._geoId; });
        assert(mesh != _meshes.end());

        auto& devContext = *context._context;
        devContext.Bind(_indexBuffer, Metal::NativeFormat::Enum(mesh->_indexFormat), mesh->_ibOffset);
        devContext.Bind(ResourceList<Metal::VertexBuffer, 1>(std::make_tuple(std::ref(_vertexBuffer))), mesh->_vertexStride, mesh->_vbOffset);

        return mesh->_techniqueInterface;
    }

    auto ModelRenderer::Pimpl::BeginSkinCall(
        const ModelRendererContext& context,
        Metal::ConstantBuffer&      localTransformBuffer,
        const MeshToModel*          transforms,
        const Float4x4&             modelToWorld,
        unsigned                    geoCallIndex,
        PreparedAnimation*          preparedAnimation) const -> TechniqueInterface
    {
        auto& cmdStream = _scaffold->CommandStream();
        auto& geoCall = cmdStream.GetSkinCall(geoCallIndex);

            // We only need to use the "transforms" array when we don't 
            // have prepared animation
            //  (otherwise that information gets burned into the 
            //   prepared vertex positions)
        if (!preparedAnimation && transforms) {
            auto meshToWorld = Combine(transforms->GetMeshToModel(geoCall._transformMarker), modelToWorld);
            auto trans = Techniques::MakeLocalTransform(modelToWorld, ExtractTranslation(context._parserContext->GetProjectionDesc()._cameraToWorld));
            localTransformBuffer.Update(*context._context, &trans, sizeof(trans));
        }

        auto cm = FindIf(_skinnedMeshes, [=](const Pimpl::SkinnedMesh& mesh) { return mesh._id == geoCall._geoId; });
        assert(cm != _skinnedMeshes.end());
        auto meshIndex = std::distance(_skinnedMeshes.cbegin(), cm);

        auto result = cm->_skinnedTechniqueInterface;

        auto& devContext = *context._context;
        devContext.Bind(_indexBuffer, Metal::NativeFormat::Enum(cm->_indexFormat), cm->_ibOffset);

        auto animGeo = SkinnedMesh::VertexStreams::AnimatedGeo;
        UINT strides[2], offsets[2];
        ID3D::Buffer* underlyingVBs[2];
        strides[0] = cm->_extraVbStride[animGeo];
        offsets[0] = cm->_extraVbOffset[animGeo];
        strides[1] = cm->_vertexStride;
        offsets[1] = cm->_vbOffset;
        underlyingVBs[0] = underlyingVBs[1] = _vertexBuffer.GetUnderlying();

            //  If we have a prepared animation, we have to replace the bindings
            //  with the data from there.
        if (preparedAnimation) {
            underlyingVBs[0] = preparedAnimation->_skinningBuffer.GetUnderlying();
            strides[0] = _skinnedBindings[meshIndex]._vertexStride;
            offsets[0] = preparedAnimation->_vbOffsets[meshIndex];
            result = _skinnedBindings[meshIndex]._techniqueInterface;
        }

        context._context->GetUnderlying()->IASetVertexBuffers(0, 2, underlyingVBs, strides, offsets);

        return result;
    }

    void ModelRenderer::Pimpl::ApplyBoundUnforms(
        const ModelRendererContext&     context,
        Metal::BoundUniforms&           boundUniforms,
        unsigned                        resourcesIndex,
        unsigned                        constantsIndex,
        const Metal::ConstantBuffer*    cbs[2])
    {
        const Metal::ShaderResourceView* srvs[16];
        assert(_texturesPerMaterial <= dimof(srvs));
        for (unsigned c=0; c<_texturesPerMaterial; c++) {
            auto* t = _boundTextures[resourcesIndex * _texturesPerMaterial + c];
            srvs[c] = t?(&t->GetShaderResource()):nullptr;
        }
        cbs[1] = &_constantBuffers[constantsIndex];
        assert(cbs[1] && cbs[1]->GetUnderlying());
        boundUniforms.Apply(
            *context._context, context._parserContext->GetGlobalUniformsStream(),
            RenderCore::Metal::UniformsStream(nullptr, cbs, 2, srvs, _texturesPerMaterial));
    }

    auto ModelRenderer::Pimpl::BuildMesh(
        const ModelCommandStream::GeoCall& geoInst,
        const RawGeometry& geo,
        ModelConstruction::BuffersUnderConstruction& workingBuffers,
        SharedStateSet& sharedStateSet,
        const uint64 textureBindPoints[], unsigned textureBindPointsCnt,
        ModelConstruction::ParamBoxDescriptions& paramBoxDesc,
        bool normalFromSkinning) -> Mesh
    {
        Mesh result;
        result._id = geoInst._geoId;
        result._indexFormat = geo._ib._format;
        result._vertexStride = geo._vb._ia._vertexStride;
        result._geoParamBox = ModelConstruction::BuildGeoParamBox(geo._vb._ia, sharedStateSet, paramBoxDesc, normalFromSkinning);

            // (source file locators)
        result._sourceFileIBOffset = geo._ib._offset;
        result._sourceFileIBSize = geo._ib._size;
        result._sourceFileVBOffset = geo._vb._offset;
        result._sourceFileVBSize = geo._vb._size;

            // (vb, ib allocations)
        result._ibOffset = workingBuffers.AllocateIB(result._sourceFileIBSize, Metal::NativeFormat::Enum(result._indexFormat));
        result._vbOffset = workingBuffers.AllocateVB(result._sourceFileVBSize);

        Metal::InputElementDesc inputDesc[12];
        unsigned vertexElementCount = BuildLowLevelInputAssembly(
            inputDesc, dimof(inputDesc),
            geo._vb._ia._elements, geo._vb._ia._elementCount);
        result._techniqueInterface = sharedStateSet.InsertTechniqueInterface(
            inputDesc, vertexElementCount, textureBindPoints, textureBindPointsCnt);

        return result;
    }

    auto ModelRenderer::Pimpl::BuildMesh(
        const ModelCommandStream::GeoCall& geoInst,
        const BoundSkinnedGeometry& geo,
        ModelConstruction::BuffersUnderConstruction& workingBuffers,
        SharedStateSet& sharedStateSet,
        const uint64 textureBindPoints[], unsigned textureBindPointsCnt,
        ModelConstruction::ParamBoxDescriptions& paramBoxDesc) -> SkinnedMesh
    {
            // Build the mesh, starting with the same basic behaviour as 
            //  unskinned meshes.
            //  (there a sort-of "slice" here... It's a bit of a hack)

        bool skinnedNormal = ModelConstruction::HasElement(geo._animatedVertexElements._ia, "NORMAL");
        Pimpl::SkinnedMesh result;
        (Pimpl::Mesh&)result = BuildMesh(
            geoInst, (const RawGeometry&)geo, workingBuffers, sharedStateSet,
            textureBindPoints, textureBindPointsCnt,
            paramBoxDesc, skinnedNormal);

        auto animGeo = Pimpl::SkinnedMesh::VertexStreams::AnimatedGeo;
        auto skelBind = Pimpl::SkinnedMesh::VertexStreams::SkeletonBinding;
        const VertexData* vd[2];
        vd[animGeo] = &geo._animatedVertexElements;
        vd[skelBind] = &geo._skeletonBinding;

        for (unsigned c=0; c<2; ++c) {
            result._sourceFileExtraVBOffset[c] = vd[c]->_offset;
            result._sourceFileExtraVBSize[c] = vd[c]->_size;
            result._extraVbOffset[c] = workingBuffers.AllocateVB(result._sourceFileExtraVBSize[c]);
            result._extraVbStride[c] = vd[c]->_ia._vertexStride;
        }

        ////////////////////////////////////////////////////////////////////////////////
            //  Build the input assembly we will use while rendering. This should 
            //  contain the unskinned the vertex elements, and also the skinned vertex
            //  elements.
            //
            //  There are 3 possible paths for the skinned vertex elements:
            //      1) we do the skinning in the vertex shader, as we encounter them.
            //          (In this case, we also need the skinning parameter vertex elements)
            //      2) we do the skinning in a geometry shader prepare step
            //      3) we do no skinning at all
            //
            //  In path 2, the geometry shader prepare step may change the format of the
            //  vertex elements. This typically occurs when using 16 bit floats (or maybe 
            //  even fixed point formats). That means we need another technique interface
            //  for the prepared animation case!
        {
            Metal::InputElementDesc inputDescForRender[12];
            unsigned eleCount = 
                BuildLowLevelInputAssembly(
                    inputDescForRender, dimof(inputDescForRender),
                    geo._animatedVertexElements._ia._elements, 
                    geo._animatedVertexElements._ia._elementCount);

                // (add the unanimated part)
            eleCount += 
                BuildLowLevelInputAssembly(
                    &inputDescForRender[eleCount], dimof(inputDescForRender) - eleCount,
                    geo._vb._ia._elements, geo._vb._ia._elementCount, 1);

            result._skinnedTechniqueInterface = sharedStateSet.InsertTechniqueInterface(
                inputDescForRender, eleCount, 
                textureBindPoints, textureBindPointsCnt);
        }

        return result;
    }

    ModelRenderer::Pimpl::DrawCallResources::DrawCallResources()
    {
        _shaderName = _geoParamBox = _materialParamBox = 0;
        _textureSet = _constantBuffer = _renderStateSet = 0;
        _delayStep = DelayStep::OpaqueRender;
        _materialBindingGuid = 0;
    }

    ModelRenderer::Pimpl::DrawCallResources::DrawCallResources(
        unsigned shaderName,
        unsigned geoParamBox, unsigned matParamBox,
        unsigned textureSet, unsigned constantBuffer,
        unsigned renderStateSet, DelayStep delayStep, MaterialGuid materialBindingGuid)
    {
        _shaderName = shaderName;
        _geoParamBox = geoParamBox;
        _materialParamBox = matParamBox;
        _textureSet = textureSet;
        _constantBuffer = constantBuffer;
        _renderStateSet = renderStateSet;
        _delayStep = delayStep;
        _materialBindingGuid = materialBindingGuid;
    }

    void    ModelRenderer::Render(
            const ModelRendererContext& context,
            const SharedStateSet&   sharedStateSet,
            const Float4x4&     modelToWorld,
            const MeshToModel*  transforms,
            PreparedAnimation*  preparedAnimation) const
    {
        auto& box = Techniques::FindCachedBox<ModelRenderingBox>(ModelRenderingBox::Desc());
        const Metal::ConstantBuffer* pkts[] = { &box._localTransformBuffer, nullptr };

        unsigned currTextureSet = ~unsigned(0x0), currCB = ~unsigned(0x0), currGeoCall = ~unsigned(0x0);
        Pimpl::TechniqueInterface currTechniqueInterface = ~Pimpl::TechniqueInterface(0x0);
        Metal::BoundUniforms* currUniforms = nullptr;
        auto& devContext = *context._context;
        auto& scaffold = *_pimpl->_scaffold;
        auto& cmdStream = scaffold.CommandStream();

        if (!transforms) {
            Techniques::LocalTransformConstants trans;
            trans = Techniques::MakeLocalTransform(modelToWorld, ExtractTranslation(context._parserContext->GetProjectionDesc()._cameraToWorld));
            box._localTransformBuffer.Update(*context._context, &trans, sizeof(trans));
        }

        if (Tweakable("SkinnedAsStatic", false)) { preparedAnimation = nullptr; }

        TRY
        {

                // skinned and unskinned geometry are almost the same, except for
                // "BeginGeoCall" / "BeginSkinCall". Never the less, we need to split
                // them into separate loops

                //////////// Render un-skinned geometry ////////////

            Metal::ConstantBuffer drawCallIndexBuffer(nullptr, sizeof(unsigned)*4);
            devContext.BindGS(MakeResourceList(drawCallIndexBuffer));

            unsigned drawCallIndex = 0;
            for (auto md=_pimpl->_drawCalls.cbegin(); md!=_pimpl->_drawCalls.cend(); ++md, ++drawCallIndex) {

                if (md->first != currGeoCall) {
                    currTechniqueInterface = _pimpl->BeginGeoCall(
                        context, box._localTransformBuffer, transforms, modelToWorld, md->first);
                    currGeoCall = md->first;
                }

                auto* boundUniforms = _pimpl->BeginVariation(context, sharedStateSet, drawCallIndex, currTechniqueInterface);
                const auto& drawCallRes = _pimpl->_drawCallRes[drawCallIndex];

                if (    boundUniforms != currUniforms 
                    ||  drawCallRes._textureSet != currTextureSet 
                    ||  drawCallRes._constantBuffer != currCB) {

                    if (boundUniforms) {
                        _pimpl->ApplyBoundUnforms(
                            context, *boundUniforms, drawCallRes._textureSet, drawCallRes._constantBuffer, pkts);
                    }

                    currTextureSet = drawCallRes._textureSet; currCB = drawCallRes._constantBuffer;
                    currUniforms = boundUniforms;
                }
            
                const auto& d = md->second;
                devContext.Bind(Metal::Topology::Enum(d._topology));  // do we really need to set the topology every time?

                    // -- this draw call index stuff is only required in some cases --
                    //      we need some way to customise the model rendering method for different purposes
                devContext.Bind(Techniques::CommonResources()._dssReadWriteWriteStencil, 1+drawCallIndex);  // write stencil buffer with draw index
                unsigned drawCallIndexB[4] = { drawCallIndex, 0, 0, 0 };
                drawCallIndexBuffer.Update(devContext, drawCallIndexB, sizeof(drawCallIndexB));
                    // -------------

                devContext.DrawIndexed(d._indexCount, d._firstIndex, d._firstVertex);
            }


                //////////// Render skinned geometry ////////////

            currGeoCall = ~unsigned(0x0);
            for (auto md=_pimpl->_skinnedDrawCalls.cbegin(); 
                md!=_pimpl->_skinnedDrawCalls.cend(); ++md, ++drawCallIndex) {

                if (md->first != currGeoCall) {
                    currTechniqueInterface = _pimpl->BeginSkinCall(
                        context, box._localTransformBuffer, transforms, modelToWorld, md->first, 
                        preparedAnimation);
                    currGeoCall = md->first;
                }

                auto* boundUniforms = _pimpl->BeginVariation(context, sharedStateSet, drawCallIndex, currTechniqueInterface);
                const auto& drawCallRes = _pimpl->_drawCallRes[drawCallIndex];

                if (    boundUniforms != currUniforms 
                    ||  drawCallRes._textureSet != currTextureSet 
                    ||  drawCallRes._constantBuffer != currCB) {

                    if (boundUniforms) {
                        _pimpl->ApplyBoundUnforms(
                            context, *boundUniforms, drawCallRes._textureSet, drawCallRes._constantBuffer, pkts);
                    }

                    currTextureSet = drawCallRes._textureSet; currCB = drawCallRes._constantBuffer;
                    currUniforms = boundUniforms;
                }
            
                const auto& d = md->second;
                devContext.Bind(Metal::Topology::Enum(d._topology));  // do we really need to set the topology every time?
                devContext.DrawIndexed(d._indexCount, d._firstIndex, d._firstVertex);
            }

        } 
        CATCH(::Assets::Exceptions::InvalidAsset& e) { context._parserContext->Process(e); }
        CATCH(::Assets::Exceptions::PendingAsset& e) { context._parserContext->Process(e); }
        CATCH_END
    }

////////////////////////////////////////////////////////////////////////////////

    bool CompareDrawCall(const DelayedDrawCall& lhs, const DelayedDrawCall& rhs)
    {
        if (lhs._shaderVariationHash == rhs._shaderVariationHash) {
            if (lhs._renderer == rhs._renderer) {
                if (lhs._subMesh == rhs._subMesh) {
                    return lhs._drawCallIndex < rhs._drawCallIndex;
                }
                return lhs._subMesh < rhs._subMesh;
            }
            return lhs._renderer < rhs._renderer;
        }
        return lhs._shaderVariationHash < rhs._shaderVariationHash; 
    }

    void    ModelRenderer::Prepare(
        DelayedDrawCallSet& dest, 
        const SharedStateSet& sharedStateSet, 
        const Float4x4& modelToWorld,
        const MeshToModel* transforms)
    {
        unsigned mainTransformIndex = ~unsigned(0x0);
        if (!transforms) {
            mainTransformIndex = (unsigned)dest._transforms.size();
            dest._transforms.push_back(modelToWorld);
        }

            //  After culling; submit all of the draw-calls in this mesh to a list to be sorted
            //  Note -- only unskinned geometry supported currently. In theory, we might be able
            //          to do the same with skinned geometry (at least, when not using the "prepare" step
        unsigned drawCallIndex = 0;
        for (auto md=_pimpl->_drawCalls.cbegin(); md!=_pimpl->_drawCalls.cend(); ++md, ++drawCallIndex) {
            const auto& drawCallRes = _pimpl->_drawCallRes[drawCallIndex];
            const auto& d = md->second;

            auto geoParamIndex = drawCallRes._geoParamBox;
            auto matParamIndex = drawCallRes._materialParamBox;
            auto shaderNameIndex = drawCallRes._shaderName;

            auto& cmdStream = _pimpl->_scaffold->CommandStream();
            auto& geoCall = cmdStream.GetGeoCall(md->first);
            auto mesh = FindIf(
                _pimpl->_meshes, [=](const Pimpl::Mesh& mesh) 
                { return mesh._id == geoCall._geoId; });
            assert(mesh != _pimpl->_meshes.end());

            auto step = unsigned(drawCallRes._delayStep);

            DelayedDrawCall entry;
            entry._drawCallIndex = drawCallIndex;
            entry._renderer = this;
            if (transforms) {
                auto trans = Combine(
                    transforms->GetMeshToModel(geoCall._transformMarker), 
                    modelToWorld);
                entry._meshToWorld = (unsigned)dest._transforms.size();
                dest._transforms.push_back(trans);
            } else {
                entry._meshToWorld = mainTransformIndex;
            }
            unsigned techniqueInterface = mesh->_techniqueInterface;
            entry._shaderVariationHash = techniqueInterface ^ (geoParamIndex << 12) ^ (matParamIndex << 15) ^ (shaderNameIndex << 24);  // simple hash of these indices. Note that collisions might be possible
            entry._indexCount = d._indexCount;
            entry._firstIndex = d._firstIndex;
            entry._firstVertex = d._firstVertex;
            entry._topology = Metal::Topology::Enum(d._topology);
            entry._subMesh = AsPointer(mesh);
            dest._entries[step].push_back(entry);
        }

            //  Also try to render skinned geometry... But we want to render this with skinning disabled 
            //  (this path is intended for rendering many static objects)
        for (auto md=_pimpl->_skinnedDrawCalls.cbegin(); md!=_pimpl->_skinnedDrawCalls.cend(); ++md, ++drawCallIndex) {
            const auto& drawCallRes = _pimpl->_drawCallRes[drawCallIndex];
            const auto& d = md->second;

            auto geoParamIndex = drawCallRes._geoParamBox;
            auto matParamIndex = drawCallRes._materialParamBox;
            auto shaderNameIndex = drawCallRes._shaderName;

            auto& cmdStream = _pimpl->_scaffold->CommandStream();
            auto& geoCall = cmdStream.GetSkinCall(md->first);
            auto mesh = FindIf(
                _pimpl->_skinnedMeshes, 
                [=](const Pimpl::Mesh& mesh) { return mesh._id == geoCall._geoId; });
            assert(mesh != _pimpl->_skinnedMeshes.end());

            auto step = unsigned(drawCallRes._delayStep);

            DelayedDrawCall entry;
            entry._drawCallIndex = drawCallIndex;
            entry._renderer = this;
            if (transforms) {
                auto trans = Combine(
                    transforms->GetMeshToModel(geoCall._transformMarker), 
                    modelToWorld);
                entry._meshToWorld = (unsigned)dest._transforms.size();
                dest._transforms.push_back(trans);
            } else {
                entry._meshToWorld = mainTransformIndex;
            }
            unsigned techniqueInterface = mesh->_skinnedTechniqueInterface;
            entry._shaderVariationHash = techniqueInterface ^ (geoParamIndex << 12) ^ (matParamIndex << 15) ^ (shaderNameIndex << 24);  // simple hash of these indices. Note that collisions might be possible
            entry._indexCount = d._indexCount;
            entry._firstIndex = d._firstIndex;
            entry._firstVertex = d._firstVertex;
            entry._topology = Metal::Topology::Enum(d._topology) | 0x100;
            entry._subMesh = AsPointer(mesh);
            dest._entries[step].push_back(entry);
        }
    }

    namespace WLTFlags { enum Enum { LocalToWorld = 1<<0, LocalSpaceView = 1<<1, MaterialGuid = 1<<2 }; }

    template<int Flags>
        void WriteLocalTransform(
            void* dest, 
            const ModelRendererContext& context, 
            const Float4x4& t, uint64 materialGuid)
    {
        auto* dst = (Techniques::LocalTransformConstants*)dest;

            //  Write some system constants that are supposed
            //  to be provided by the model renderer
        if (constant_expression<!!(Flags&WLTFlags::LocalToWorld)>::result()) {
            CopyTransform(dst->_localToWorld, t);
        }
        if (constant_expression<!!(Flags&WLTFlags::LocalSpaceView)>::result()) {
            auto worldSpaceView = ExtractTranslation(context._parserContext->GetProjectionDesc()._cameraToWorld);
            TransformPointByOrthonormalInverse(t, worldSpaceView);
            dst->_localSpaceView = worldSpaceView;
        }
        if (constant_expression<!!(Flags&WLTFlags::MaterialGuid)>::result()) {
            dst->_materialGuid = materialGuid;
        }
    }

    template<bool HasCallback>
        void ModelRenderer::RenderPreparedInternal(
            const ModelRendererContext& context, const SharedStateSet& sharedStateSet,
            DelayedDrawCallSet& drawCalls, DelayStep delayStep,
            const std::function<void(unsigned, unsigned, unsigned)>* callback)
    {
        if (drawCalls.GetRendererGUID() != typeid(ModelRenderer).hash_code())
            Throw(::Exceptions::BasicLabel("Delayed draw call set matched with wrong renderer type"));

        auto& entries = drawCalls._entries[(unsigned)delayStep];
        if (entries.empty()) return;

        Techniques::LocalTransformConstants localTrans;
        localTrans._localSpaceView = Float3(0.f, 0.f, 0.f);
        
        Metal::ConstantBuffer& localTransformBuffer = Techniques::CommonResources()._localTransformBuffer;
        const Metal::ConstantBuffer* pkts[] = { &localTransformBuffer, nullptr };

        std::sort(entries.begin(), entries.end(), CompareDrawCall);

        const ModelRenderer::Pimpl::Mesh* currentMesh = nullptr;
        RenderCore::Metal::BoundUniforms* boundUniforms = nullptr;
        unsigned currentVariationHash = ~unsigned(0x0);
        unsigned currentTextureSet = ~unsigned(0x0);
        unsigned currentConstantBufferIndex = ~unsigned(0x0);
        unsigned currentTechniqueInterface = ~unsigned(0x0);

        for (auto d=entries.cbegin(); d!=entries.cend(); ++d) {
            auto& renderer = *(const ModelRenderer*)d->_renderer;
            const auto& drawCallRes = renderer._pimpl->_drawCallRes[d->_drawCallIndex];

            if (currentMesh != d->_subMesh) {
                if (d->_topology > 0xff) {
                    auto& mesh = *(const Pimpl::SkinnedMesh*)d->_subMesh;
                    context._context->Bind(renderer._pimpl->_indexBuffer, Metal::NativeFormat::Enum(mesh._indexFormat), mesh._ibOffset);

                    const Metal::VertexBuffer* vbs[] = { &renderer._pimpl->_vertexBuffer, &renderer._pimpl->_vertexBuffer };
                    unsigned strides[] = { mesh._extraVbStride[0], mesh._vertexStride };
                    unsigned offsets[] = { mesh._extraVbOffset[0], mesh._vbOffset };
                    context._context->Bind(0, 2, vbs, strides, offsets);
                    currentMesh = &mesh;

                    currentTechniqueInterface = mesh._skinnedTechniqueInterface;
                } else {
                    auto& mesh = *(const Pimpl::Mesh*)d->_subMesh;
                    context._context->Bind(renderer._pimpl->_indexBuffer, Metal::NativeFormat::Enum(mesh._indexFormat), mesh._ibOffset);
                    context._context->Bind(MakeResourceList(renderer._pimpl->_vertexBuffer), mesh._vertexStride, mesh._vbOffset);
                    currentMesh = &mesh;
                    currentTechniqueInterface = mesh._techniqueInterface;
                }
                currentTextureSet = ~unsigned(0x0);
            }

                // Note -- at the moment, shader variation hash is the sorting priority.
                //          This reduces the shader changes to a minimum. It also means we
                //          do the work in "BeginVariation" to resolve the variation
                //          as rarely as possible. However, we could pre-resolve all of the
                //          variations that we're going to need and use another value as the
                //          sorting priority instead... That might reduce the API thrashing
                //          in some cases.
            if (currentVariationHash != d->_shaderVariationHash) {
                auto& mesh = *(const Pimpl::Mesh*)d->_subMesh;
                boundUniforms = sharedStateSet.BeginVariation(
                    context, drawCallRes._shaderName, currentTechniqueInterface, drawCallRes._geoParamBox, 
                    drawCallRes._materialParamBox);
                currentVariationHash = d->_shaderVariationHash;
                currentTextureSet = ~unsigned(0x0);
            }

            if (!boundUniforms) continue;

            static Utility::ParameterBox tempGlobalStatesBox;
            sharedStateSet.BeginRenderState(context, tempGlobalStatesBox, drawCallRes._renderStateSet);

                // We have to do this transform update very frequently! isn't there a better way?
            {
                D3D11_MAPPED_SUBRESOURCE result;
                HRESULT hresult = context._context->GetUnderlying()->Map(
                    localTransformBuffer.GetUnderlying(), 0, D3D11_MAP_WRITE_DISCARD, 0, &result);
                assert(SUCCEEDED(hresult) && result.pData); (void)hresult;
                WriteLocalTransform<WLTFlags::LocalToWorld|WLTFlags::MaterialGuid>(
                    result.pData, context, drawCalls._transforms[d->_meshToWorld], drawCallRes._materialBindingGuid);
                context._context->GetUnderlying()->Unmap(localTransformBuffer.GetUnderlying(), 0);
            }
            
            auto textureSet = drawCallRes._textureSet;
            auto constantBufferIndex = drawCallRes._constantBuffer;

                //  sometimes the same render call may be rendered in several different locations. In these cases,
                //  we can reduce the API thrashing to the minimum by avoiding re-setting resources and constants
            if (boundUniforms && (textureSet != currentTextureSet || constantBufferIndex != currentConstantBufferIndex)) {
                renderer._pimpl->ApplyBoundUnforms(
                    context, *boundUniforms,
                    textureSet, constantBufferIndex, pkts);

                currentTextureSet = textureSet;
                currentConstantBufferIndex = constantBufferIndex;
            }

            context._context->Bind((Metal::Topology::Enum)(d->_topology & 0xff));
            if (constant_expression<HasCallback>::result()) {
                (*callback)(d->_indexCount, d->_firstIndex, d->_firstVertex);
            } else
                context._context->DrawIndexed(d->_indexCount, d->_firstIndex, d->_firstVertex);
        }
    }

    void ModelRenderer::RenderPrepared(
        const ModelRendererContext& context, const SharedStateSet& sharedStateSet,
        DelayedDrawCallSet& drawCalls, DelayStep delayStep)
    {
        RenderPreparedInternal<false>(context, sharedStateSet, drawCalls, delayStep, nullptr);
    }

    void ModelRenderer::RenderPrepared(
        const ModelRendererContext& context, const SharedStateSet& sharedStateSet,
        DelayedDrawCallSet& drawCalls, DelayStep delayStep,
        const std::function<void(unsigned, unsigned, unsigned)>& callback)
    {
        assert(callback);
        RenderPreparedInternal<true>(context, sharedStateSet, drawCalls, delayStep, &callback);
    }

////////////////////////////////////////////////////////////////////////////////

        /// \todo --    this DestroyArray stuff is too awkward. Instead, let's create
        ///             some "SerializeableArray" or "SerializeableBlock" or something --
        ///             and have it deal with the internals (even if it means increasing memory size slightly)
    ModelCommandStream::~ModelCommandStream()
    {
        DestroyArray(_geometryInstances,        &_geometryInstances[_geometryInstanceCount]);
        DestroyArray(_skinControllerInstances,  &_skinControllerInstances[_skinControllerInstanceCount]);
    }

    GeoInputAssembly::~GeoInputAssembly()
    {
        DestroyArray(_elements, &_elements[_elementCount]);
    }

    RawGeometry::~RawGeometry() {}
    BoundSkinnedGeometry::~BoundSkinnedGeometry() {}

    ModelImmutableData::~ModelImmutableData()
    {
        DestroyArray(_geos, &_geos[_geoCount]);
        DestroyArray(_boundSkinnedControllers, &_boundSkinnedControllers[_boundSkinnedControllerCount]);
    }

        ////////////////////////////////////////////////////////////

    uint64 GeoInputAssembly::BuildHash() const
    {
            //  Build a hash for this object.
            //  Note that we should be careful that we don't get an
            //  noise from characters in the left-over space in the
            //  semantic names. Do to this right, we should make sure
            //  that left over space has no effect.
        auto elementsHash = Hash64(_elements, PtrAdd(_elements, _elementCount * sizeof(VertexElement)));
        elementsHash ^= uint64(_vertexStride);
        return elementsHash;
    }

        ////////////////////////////////////////////////////////////

    Float4x4 ModelRenderer::MeshToModel::GetMeshToModel(unsigned transformMarker) const
    {
            //  The "skeleton binding" tells us how to map from the matrices that
            //  are output from the transformation machine to the input matrices
            //  expected by the "transformMarker" index scheme
        if (_skeletonBinding) {
            assert(transformMarker < _skeletonBinding->_modelJointIndexToMachineOutput.size());
            auto machineOutputIndex = _skeletonBinding->_modelJointIndexToMachineOutput[transformMarker];
            if (machineOutputIndex == ~unsigned(0x0)) {
                    // no mapping... we just have to assume identity mesh-to-model
                return Identity<Float4x4>();
            } else if (machineOutputIndex >= _skeletonOutputCount) {
                assert(0);
                return Identity<Float4x4>();
            }
            return _skeletonOutput[machineOutputIndex];
        } else {            
            if (transformMarker >= _skeletonOutputCount) {
                assert(0);
                return Identity<Float4x4>();
            }
            return _skeletonOutput[transformMarker];
        }
    }

    ModelRenderer::MeshToModel::MeshToModel()
    {
        _skeletonOutput = nullptr;
        _skeletonOutputCount = 0;
        _skeletonBinding = nullptr;
    }

    ModelRenderer::MeshToModel::MeshToModel(
        const Float4x4 skeletonOutput[], unsigned skeletonOutputCount,
        const SkeletonBinding* binding)
    {
        _skeletonOutput = skeletonOutput;
        _skeletonOutputCount = skeletonOutputCount;
        _skeletonBinding = binding;
    }

    template<unsigned Size>
        static std::string Width(unsigned input)
    {
        static char buffer[Size+1];
        auto err = _itoa_s(input, buffer, 10);
        if (!err) {
            auto length = XlStringLen(buffer);
            if (length < Size) {
                auto movement = Size-length;
                XlMoveMemory(&buffer[movement], buffer, length);
                XlSetMemory(buffer, ' ', movement);
                buffer[Size] = '\0';
            }
            return std::string(buffer);
        } else { return std::string("<<err>>"); }
    }

    std::vector<MaterialGuid> ModelRenderer::DrawCallToMaterialBinding() const
    {
        std::vector<MaterialGuid> result;
        result.reserve(_pimpl->_drawCallRes.size());
        for (auto i=_pimpl->_drawCallRes.begin(); i!=_pimpl->_drawCallRes.end(); ++i) {
            result.push_back(i->_materialBindingGuid);
        }
        return std::move(result);
    }

    MaterialGuid ModelRenderer::GetMaterialBindingForDrawCall(unsigned drawCallIndex) const
    {
        if (drawCallIndex < _pimpl->_drawCallRes.size())
            return _pimpl->_drawCallRes[drawCallIndex]._materialBindingGuid;
        return ~0ull;
    }

    void ModelRenderer::LogReport() const
    {
        LogInfo << "---<< Model Renderer: " << _pimpl->_scaffold->Filename() << " (LOD: " << _pimpl->_levelOfDetail << ") >>---";
        LogInfo << "  [" << _pimpl->_meshes.size() << "] meshes";
        LogInfo << "  [" << _pimpl->_skinnedMeshes.size() << "] skinned meshes";
        LogInfo << "  [" << _pimpl->_constantBuffers.size() << "] constant buffers";
        LogInfo << "  [" << _pimpl->_drawCalls.size() << "] draw calls";
        LogInfo << "  [" << _pimpl->_skinnedDrawCalls.size() << "] skinned draw calls";
        LogInfo << "  [" << _pimpl->_boundTextures.size() << "] bound textures";
        LogInfo << "  [" << _pimpl->_texturesPerMaterial << "] textures per material";
        DEBUG_ONLY(LogInfo << "  [" << _pimpl->_vbSize / 1024.f << "k] VB size");
        DEBUG_ONLY(LogInfo << "  [" << _pimpl->_ibSize / 1024.f << "k] IB size");
        LogInfo << "  Draw calls |  Indxs | GeoC |  Shr | GeoP | MatP |  Tex |   CB |   RS ";

        for (unsigned c=0; c<_pimpl->_drawCalls.size(); ++c) {
            const auto&m = _pimpl->_drawCalls[c].first;
            const auto&d = _pimpl->_drawCalls[c].second;
            const auto&r = _pimpl->_drawCallRes[c];
            LogInfo
                << "  [" << Width<3>(c) << "] (M)  |"
                << Width<7>(d._indexCount) << " |"
                << Width<5>(m) << " |"
                << Width<5>(r._shaderName) << " |"
                << Width<5>(r._geoParamBox) << " |"
                << Width<5>(r._materialParamBox) << " |"
                << Width<5>(r._textureSet) << " |"
                << Width<5>(r._constantBuffer) << " |"
                << Width<5>(r._renderStateSet);
        }

        for (unsigned c=0; c<_pimpl->_skinnedDrawCalls.size(); ++c) {
            const auto&m = _pimpl->_skinnedDrawCalls[c].first;
            const auto&d = _pimpl->_skinnedDrawCalls[c].second;
            const auto&r = _pimpl->_drawCallRes[c + _pimpl->_drawCalls.size()];
            LogInfo
                << "  [" << Width<3>(c) << "] (S)  |"
                << Width<7>(d._indexCount) << " |"
                << Width<5>(m) << " |"
                << Width<5>(r._shaderName) << " |"
                << Width<5>(r._geoParamBox) << " |"
                << Width<5>(r._materialParamBox) << " |"
                << Width<5>(r._textureSet) << " |"
                << Width<5>(r._constantBuffer) << " |"
                << Width<5>(r._renderStateSet);
        }

        LogInfo << "  Meshes     | GeoC |  SrcVB |  SrcIB | VtxS | TchI | GeoP | IdxF";

        for (unsigned c=0; c<_pimpl->_meshes.size(); ++c) {
            const auto&m = _pimpl->_meshes[c];
            LogInfo
                << "  [" << Width<3>(c) << "] (M)  |"
                << Width<5>(m._id) << " |"
                << Width<6>(m._sourceFileVBSize/1024) << "k |"
                << Width<6>(m._sourceFileIBSize/1024) << "k |"
                << Width<5>(m._vertexStride) << " |"
                << Width<5>(m._techniqueInterface) << " |"
                << Width<5>(m._geoParamBox) << " |"
                << Width<5>(m._indexFormat);
        }
        for (unsigned c=0; c<_pimpl->_skinnedMeshes.size(); ++c) {
            const auto&m = _pimpl->_skinnedMeshes[c];
            LogInfo
                << "  [" << Width<3>(c) << "] (S)  |"
                << Width<5>(m._id) << " |"
                << Width<6>(m._sourceFileVBSize/1024) << "k |"
                << Width<6>(m._sourceFileIBSize/1024) << "k |"
                << Width<5>(m._vertexStride) << " |"
                << Width<5>(m._techniqueInterface) << " |"
                << Width<5>(m._geoParamBox) << " |"
                << Width<5>(m._indexFormat);
        }

        #if defined(_DEBUG)
            if (_pimpl->_texturesPerMaterial) {
                LogInfo << "  Bound Textures";
                for (unsigned c=0; c<_pimpl->_boundTextureNames.size() / _pimpl->_texturesPerMaterial; ++c) {
                    StringMeld<512> temp;
                    for (unsigned q=0; q<_pimpl->_texturesPerMaterial; ++q) {
                        if (q) { temp << ", "; }
                        temp << _pimpl->_boundTextureNames[c*_pimpl->_texturesPerMaterial+q];
                    }
                    LogInfo << "  [" << Width<3>(c) << "] " << temp;
                }
            }

            LogInfo << "  Parameter Boxes";
            for (unsigned c=0; c<_pimpl->_paramBoxDesc.size(); ++c) {
                auto& i = _pimpl->_paramBoxDesc[c];
                LogInfo << "  [" << Width<3>(i.first) << "] " << i.second;
            }
        #endif
    }

        ////////////////////////////////////////////////////////////

    static std::pair<std::unique_ptr<uint8[]>, unsigned> LoadRawData(const char filename[])
    {
        BasicFile file(filename, "rb");
        auto chunks = Serialization::ChunkFile::LoadChunkTable(file);

            // look for the first model scaffold chunk
        Serialization::ChunkFile::ChunkHeader largeBlocksChunk;
        Serialization::ChunkFile::ChunkHeader scaffoldChunk;

        for (auto i=chunks.begin(); i!=chunks.end(); ++i) {
            if (i->_type == ChunkType_ModelScaffold && !scaffoldChunk._fileOffset) {
                scaffoldChunk = *i;
            }
            if (i->_type == ChunkType_ModelScaffoldLargeBlocks && !largeBlocksChunk._fileOffset) {
                largeBlocksChunk = *i;
            }
        }

        if (    !scaffoldChunk._fileOffset
            ||  !largeBlocksChunk._fileOffset) {
            throw ::Assets::Exceptions::FormatError("Missing model scaffold chunks: %s", filename);
        }

        if (scaffoldChunk._chunkVersion != 0) {
            throw ::Assets::Exceptions::FormatError("Incorrect file version: %s", filename);
        }

        auto rawMemoryBlock = std::make_unique<uint8[]>(scaffoldChunk._size);
        file.Seek(scaffoldChunk._fileOffset, SEEK_SET);
        file.Read(rawMemoryBlock.get(), 1, scaffoldChunk._size);

        return std::make_pair(std::move(rawMemoryBlock), largeBlocksChunk._fileOffset);
    }

    ModelScaffold::ModelScaffold(const ResChar filename[])
    {
        std::unique_ptr<uint8[]> rawMemoryBlock;
        unsigned largeBlocksOffset = 0;
        std::tie(rawMemoryBlock, largeBlocksOffset) = LoadRawData(filename);
        
        Serialization::Block_Initialize(rawMemoryBlock.get());        
        _data = (const ModelImmutableData*)Serialization::Block_GetFirstObject(rawMemoryBlock.get());

        _validationCallback = std::make_shared<::Assets::DependencyValidation>();
        RegisterFileDependency(_validationCallback, filename);
        
        _filename = filename;
        _rawMemoryBlock = std::move(rawMemoryBlock);
        _largeBlocksOffset = largeBlocksOffset;
    }
    
    ModelScaffold::ModelScaffold(std::shared_ptr<::Assets::PendingCompileMarker>&& marker)
    {
        _data = nullptr;
        _largeBlocksOffset = 0;

        if (marker->GetState() == ::Assets::AssetState::Ready) {
            CompleteFromMarker(*marker);
        } else {
            _marker = std::forward<std::shared_ptr<::Assets::PendingCompileMarker>>(marker);
            _validationCallback = std::make_shared<::Assets::DependencyValidation>();
        }
    }

    ModelScaffold::ModelScaffold(ModelScaffold&& moveFrom)
    : _rawMemoryBlock(std::move(moveFrom._rawMemoryBlock))
    , _filename(std::move(moveFrom._filename))
    , _marker(std::move(moveFrom._marker))
    , _validationCallback(std::move(moveFrom._validationCallback))
    {
        _data = moveFrom._data;
        moveFrom._data = nullptr;
        _largeBlocksOffset = moveFrom._largeBlocksOffset;
    }

    ModelScaffold& ModelScaffold::operator=(ModelScaffold&& moveFrom)
    {
        _rawMemoryBlock = std::move(moveFrom._rawMemoryBlock);
        _data = moveFrom._data;
        moveFrom._data = nullptr;
        _largeBlocksOffset = moveFrom._largeBlocksOffset;
        _filename = std::move(moveFrom._filename);
        _marker = std::move(moveFrom._marker);
        _validationCallback = std::move(moveFrom._validationCallback);
        return *this;
    }

    ModelScaffold::~ModelScaffold()
    {
        if (_data)
            _data->~ModelImmutableData();
    }

    void ModelScaffold::Resolve() const
    {
        if (_marker) {
            if (_marker->GetState() == ::Assets::AssetState::Invalid) {
                Throw(::Assets::Exceptions::InvalidAsset(_marker->Initializer(), ""));
            } else if (_marker->GetState() == ::Assets::AssetState::Pending) {
                    // we need to throw immediately on pending resource
                    // this object is useless while it's pending.
                Throw(::Assets::Exceptions::PendingAsset(_marker->Initializer(), ""));
            }

                // hack --  Resolve needs to be called by const methods (like "GetStaticBoundingBox")
                //          but Resolve() must change all the internal pointers... It's an awkward
                //          case for const-correctness
            const_cast<ModelScaffold*>(this)->CompleteFromMarker(*_marker);
            _marker.reset();
        }
    }

    ::Assets::AssetState ModelScaffold::TryResolve()
    {
        if (_marker) {
            auto markerState = _marker->GetState();
            if (markerState != ::Assets::AssetState::Ready) return markerState;

            CompleteFromMarker(*_marker);
            _marker.reset();
        }

        return ::Assets::AssetState::Ready;
    }

    ::Assets::AssetState ModelScaffold::StallAndResolve()
    {
        if (_marker) {
            auto markerState = _marker->StallWhilePending();
            if (markerState != ::Assets::AssetState::Ready) return markerState;

            CompleteFromMarker(*_marker);
            _marker.reset();
        }

        return ::Assets::AssetState::Ready;
    }

    void ModelScaffold::CompleteFromMarker(::Assets::PendingCompileMarker& marker)
    {
        std::unique_ptr<uint8[]> rawMemoryBlock;
        unsigned largeBlocksOffset = 0;

        std::tie(rawMemoryBlock, largeBlocksOffset) = LoadRawData(marker._sourceID0);

        Serialization::Block_Initialize(rawMemoryBlock.get());        
        _data = (const ModelImmutableData*)Serialization::Block_GetFirstObject(rawMemoryBlock.get());

        _filename = marker._sourceID0;
        if (!_validationCallback) {
            _validationCallback = marker._dependencyValidation;
        } else 
            ::Assets::RegisterAssetDependency(_validationCallback, marker._dependencyValidation);
        _rawMemoryBlock = std::move(rawMemoryBlock);
        _largeBlocksOffset = largeBlocksOffset;
    }

    unsigned                        ModelScaffold::LargeBlocksOffset() const            { Resolve(); return _largeBlocksOffset; }
    const ModelImmutableData&       ModelScaffold::ImmutableData() const                { Resolve(); return *_data; };
    const ModelCommandStream&       ModelScaffold::CommandStream() const                { Resolve(); return _data->_visualScene; }
    const TransformationMachine&    ModelScaffold::EmbeddedSkeleton() const             { Resolve(); return _data->_embeddedSkeleton; }
    std::pair<Float3, Float3>       ModelScaffold::GetStaticBoundingBox(unsigned) const { Resolve(); return _data->_boundingBox; }

}}

