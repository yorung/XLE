// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "LightingTargets.h"
#include "SceneEngineUtils.h"
#include "LightingParserContext.h"

#include "../BufferUploads/ResourceLocator.h"
#include "../RenderCore/Techniques/Techniques.h"
#include "../RenderCore/Techniques/CommonResources.h"
#include "../RenderCore/Metal/Shader.h"
#include "../RenderCore/Metal/DeviceContext.h"
#include "../Assets/AssetUtils.h"
#include "../Utility/PtrUtils.h"
#include "../Utility/StringFormat.h"

#include "../RenderCore/DX11/Metal/IncludeDX11.h"

namespace SceneEngine
{

    MainTargetsBox::Desc::Desc( unsigned width, unsigned height, 
                                const FormatStack& diffuseFormat, const FormatStack& normalFormat, 
                                const FormatStack& parametersFormat, const FormatStack& depthFormat,
                                const BufferUploads::TextureSamples& sampling)
    {
            //  we have to "memset" this -- because padding adds 
            //  random values in profile mode
        std::fill((char*)this, PtrAdd((char*)this, sizeof(*this)), 0);

        _width = width; _height = height;
        _gbufferFormats[0] = diffuseFormat;
        _gbufferFormats[1] = normalFormat;
        _gbufferFormats[2] = parametersFormat;
        _depthFormat = depthFormat;
        _sampling = sampling;
    }

    MainTargetsBox::MainTargetsBox(const Desc& desc) 
    : _desc(desc)
    {
        using namespace RenderCore;
        using namespace RenderCore::Metal;
        using namespace BufferUploads;

        ResourcePtr gbufferTextures[s_gbufferTextureCount];
        RenderTargetView gbufferRTV[dimof(gbufferTextures)];
        ShaderResourceView gbufferSRV[dimof(gbufferTextures)];
        std::fill(gbufferTextures, &gbufferTextures[dimof(gbufferTextures)], nullptr);

        auto bufferUploadsDesc = BufferUploads::CreateDesc(
            BindFlag::ShaderResource|BindFlag::RenderTarget,
            0, GPUAccess::Write | GPUAccess::Read,
            BufferUploads::TextureDesc::Plain2D(
                desc._width, desc._height, AsDXGIFormat(NativeFormat::Unknown), 1, 0, desc._sampling),
            "GBuffer");
        for (unsigned c=0; c<dimof(gbufferTextures); ++c) {
            if (desc._gbufferFormats[c]._resourceFormat != NativeFormat::Unknown) {
                bufferUploadsDesc._textureDesc._nativePixelFormat = AsDXGIFormat(desc._gbufferFormats[c]._resourceFormat);
                gbufferTextures[c] = CreateResourceImmediate(bufferUploadsDesc);
                gbufferRTV[c] = RenderTargetView(gbufferTextures[c].get(), desc._gbufferFormats[c]._writeFormat);
                gbufferSRV[c] = ShaderResourceView(gbufferTextures[c].get(), desc._gbufferFormats[c]._shaderReadFormat);
            }
        }

            /////////
        
        auto depthBufferDesc = BufferUploads::CreateDesc(
            BindFlag::ShaderResource|BindFlag::DepthStencil,
            0, GPUAccess::Write | GPUAccess::Read,
            BufferUploads::TextureDesc::Plain2D(
                desc._width, desc._height, AsDXGIFormat(desc._depthFormat._resourceFormat), 1, 0, desc._sampling),
            "MainDepth");
        auto msaaDepthBufferTexture = CreateResourceImmediate(depthBufferDesc);
        auto secondaryDepthBufferTexture = CreateResourceImmediate(depthBufferDesc);
        DepthStencilView msaaDepthBuffer(msaaDepthBufferTexture.get(), desc._depthFormat._writeFormat);
        DepthStencilView secondaryDepthBuffer(secondaryDepthBufferTexture.get(), desc._depthFormat._writeFormat);
        ShaderResourceView msaaDepthBufferSRV(msaaDepthBufferTexture.get(), desc._depthFormat._shaderReadFormat);
        ShaderResourceView secondaryDepthBufferSRV(secondaryDepthBufferTexture.get(), desc._depthFormat._shaderReadFormat);

            /////////

        for (unsigned c=0; c<dimof(_gbufferTextures); ++c) {
            _gbufferTextures[c] = std::move(gbufferTextures[c]);
            _gbufferRTVs[c] = std::move(gbufferRTV[c]);
            _gbufferRTVsSRV[c] = std::move(gbufferSRV[c]);
        }
        _msaaDepthBufferTexture = std::move(msaaDepthBufferTexture);
        _secondaryDepthBufferTexture = std::move(secondaryDepthBufferTexture);
        _msaaDepthBuffer = std::move(msaaDepthBuffer);
        _secondaryDepthBuffer = std::move(secondaryDepthBuffer);
        _msaaDepthBufferSRV = std::move(msaaDepthBufferSRV);
        _secondaryDepthBufferSRV = std::move(secondaryDepthBufferSRV);
    }

    MainTargetsBox::~MainTargetsBox() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ForwardTargetsBox::Desc::Desc( unsigned width, unsigned height, 
                                const FormatStack& depthFormat,
                                const BufferUploads::TextureSamples& sampling)
    {
            //  we have to "memset" this -- because padding adds random values in 
            //  profile mode
        std::fill((char*)this, PtrAdd((char*)this, sizeof(*this)), 0);

        _width = width; _height = height;
        _depthFormat = depthFormat;
        _sampling = sampling;
    }

    ForwardTargetsBox::ForwardTargetsBox(const Desc& desc) 
    : _desc(desc)
    {
        using namespace RenderCore;
        using namespace RenderCore::Metal;
        using namespace BufferUploads;
        auto bufferUploadsDesc = BuildRenderTargetDesc(
            BindFlag::ShaderResource|BindFlag::DepthStencil,
            BufferUploads::TextureDesc::Plain2D(
                desc._width, desc._height, AsDXGIFormat(desc._depthFormat._resourceFormat), 1, 0, desc._sampling),
            "ForwardTarget");

        auto msaaDepthBufferTexture = CreateResourceImmediate(bufferUploadsDesc);
        auto secondaryDepthBufferTexture = CreateResourceImmediate(bufferUploadsDesc);

            /////////

        DepthStencilView msaaDepthBuffer(msaaDepthBufferTexture.get(), desc._depthFormat._writeFormat);
        DepthStencilView secondaryDepthBuffer(secondaryDepthBufferTexture.get(), desc._depthFormat._writeFormat);

        ShaderResourceView msaaDepthBufferSRV(msaaDepthBufferTexture.get(), desc._depthFormat._shaderReadFormat);
        ShaderResourceView secondaryDepthBufferSRV(secondaryDepthBufferTexture.get(), desc._depthFormat._shaderReadFormat);

            /////////

        _msaaDepthBufferTexture = std::move(msaaDepthBufferTexture);
        _secondaryDepthBufferTexture = std::move(secondaryDepthBufferTexture);

        _msaaDepthBuffer = std::move(msaaDepthBuffer);
        _secondaryDepthBuffer = std::move(secondaryDepthBuffer);

        _msaaDepthBufferSRV = std::move(msaaDepthBufferSRV);
        _secondaryDepthBufferSRV = std::move(secondaryDepthBufferSRV);
    }

    ForwardTargetsBox::~ForwardTargetsBox() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////

    LightingResolveTextureBox::Desc::Desc( unsigned width, unsigned height, 
                                const FormatStack& lightingResolveFormat,
                                const BufferUploads::TextureSamples& sampling)
    {
            //  we have to "memset" this -- because padding adds 
            //  random values in profile mode
        std::fill((char*)this, PtrAdd((char*)this, sizeof(*this)), 0);

        _width = width; _height = height;
        _lightingResolveFormat = lightingResolveFormat;
        _sampling = sampling;
    }
    
    LightingResolveTextureBox::LightingResolveTextureBox(const Desc& desc)
    {
        using namespace RenderCore;
        using namespace RenderCore::Metal;
        using namespace BufferUploads;
        auto bufferUploadsDesc = BuildRenderTargetDesc(
            BindFlag::ShaderResource|BindFlag::RenderTarget,
            BufferUploads::TextureDesc::Plain2D(
                desc._width, desc._height, AsDXGIFormat(desc._lightingResolveFormat._resourceFormat), 1, 0, 
                desc._sampling),
            "LightResolve");

        auto lightingResolveTexture = CreateResourceImmediate(bufferUploadsDesc);
        auto lightingResolveCopy = CreateResourceImmediate(bufferUploadsDesc);
        bufferUploadsDesc._textureDesc._samples = TextureSamples::Create();

        RenderTargetView lightingResolveTarget(lightingResolveTexture.get(), desc._lightingResolveFormat._writeFormat);
        ShaderResourceView lightingResolveSRV(lightingResolveTexture.get(), desc._lightingResolveFormat._shaderReadFormat);
        ShaderResourceView lightingResolveCopySRV(lightingResolveCopy.get(), desc._lightingResolveFormat._shaderReadFormat);

        _lightingResolveTexture = std::move(lightingResolveTexture);
        _lightingResolveRTV = std::move(lightingResolveTarget);
        _lightingResolveSRV = std::move(lightingResolveSRV);

        _lightingResolveCopy = std::move(lightingResolveCopy);
        _lightingResolveCopySRV = std::move(lightingResolveCopySRV);
    }

    LightingResolveTextureBox::~LightingResolveTextureBox()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    unsigned LightingResolveShaders::LightShaderType::ReservedIndexCount()
    {
        return 0x1F + 1;
    }

    unsigned LightingResolveShaders::LightShaderType::AsIndex() const
    {
            // We must compress the information in this object down
            // into a single unique id. We want to make sure each configuration
            // produces a unique id. But ids must be (close to) contiguous, and we want to
            // reserve as few id numbers as possible.
        auto shadows = _shadows;
        if (_projection == Point && shadows == OrthShadows) { shadows = PerspectiveShadows; }
        auto shadowResolveModel = _shadowResolveModel;
        if (shadows == NoShadows) { shadowResolveModel = 0; }

        return 
              ((_projection & 0x1) << 0)
            | ((shadows & 0x3) << 1)
            | ((_diffuseModel & 0x1) << 3)
            | ((shadowResolveModel & 0x1) << 4)
            ;
    }

    void LightingResolveShaders::BuildShader(const Desc& desc, const LightShaderType& type)
    {
        using namespace RenderCore;

        StringMeld<256, ::Assets::ResChar> definesTable;
        definesTable << "GBUFFER_TYPE=" << desc._gbufferType;
        definesTable << ";MSAA_SAMPLES=" << (desc._msaaSampleCount<=1)?0:desc._msaaSampleCount;
        if (desc._msaaSamplers) definesTable << ";MSAA_SAMPLERS=1";
        definesTable << ";SHADOW_CASCADE_MODE=" << ((type._shadows == OrthShadows || type._shadows == OrthHybridShadows) ? 2u : 1u);
        definesTable << ";DIFFUSE_METHOD=" << unsigned(type._diffuseModel);
        definesTable << ";SHADOW_RESOLVE_MODEL=" << unsigned(type._shadowResolveModel);
        definesTable << ";SHADOW_RT_HYBRID=" << unsigned(type._shadows == OrthHybridShadows);

        const char* vertexShader_viewFrustumVector = 
            desc._flipDirection
                ? "game/xleres/basic2D.vsh:fullscreen_flip_viewfrustumvector:vs_*"
                : "game/xleres/basic2D.vsh:fullscreen_viewfrustumvector:vs_*"
                ;

        LightShader& dest = _shaders[type.AsIndex()];
        assert(!dest._shader);

        if (type._projection == Point) {

            if (type._shadows == NoShadows) {
                dest._shader = &::Assets::GetAssetDep<Metal::ShaderProgram>(
                    vertexShader_viewFrustumVector, 
                    "game/xleres/deferred/resolveunshadowed.psh:ResolvePointLightUnshadowed:ps_*",
                    definesTable.get());
            } else {
                dest._shader = &::Assets::GetAssetDep<Metal::ShaderProgram>(
                    vertexShader_viewFrustumVector, 
                    "game/xleres/deferred/resolve.psh:ResolvePointLight:ps_*",
			        definesTable.get());
            }

        } else if (type._projection == Directional) {

            if (type._shadows == NoShadows) {
                dest._shader = &::Assets::GetAssetDep<Metal::ShaderProgram>(
                    vertexShader_viewFrustumVector, 
                    "game/xleres/deferred/resolveunshadowed.psh:ResolveLightUnshadowed:ps_*",
                    definesTable.get());
            } else {
                dest._shader = &::Assets::GetAssetDep<Metal::ShaderProgram>(
                    vertexShader_viewFrustumVector, 
                    "game/xleres/deferred/resolve.psh:ResolveLight:ps_*",
                    definesTable.get());
            }

        }

        dest._uniforms = Metal::BoundUniforms(std::ref(*dest._shader));

        Techniques::TechniqueContext::BindGlobalUniforms(dest._uniforms);
        dest._uniforms.BindConstantBuffer(Hash64("ArbitraryShadowProjection"),  CB::ShadowProj_Arbit, 1);
        dest._uniforms.BindConstantBuffer(Hash64("LightBuffer"),                CB::LightBuffer, 1);
        dest._uniforms.BindConstantBuffer(Hash64("ShadowParameters"),           CB::ShadowParam, 1);
        dest._uniforms.BindConstantBuffer(Hash64("ScreenToShadowProjection"),   CB::ScreenToShadow, 1);
        dest._uniforms.BindConstantBuffer(Hash64("OrthogonalShadowProjection"), CB::ShadowProj_Ortho, 1);
        dest._uniforms.BindConstantBuffer(Hash64("ShadowResolveParameters"),    CB::ShadowResolveParam, 1);
        dest._uniforms.BindConstantBuffer(Hash64("ScreenToRTShadowProjection"), CB::ScreenToRTShadow, 1);
        dest._uniforms.BindShaderResource(Hash64("RTSListsHead"),               SR::RTShadow_ListHead, 1);
        dest._uniforms.BindShaderResource(Hash64("RTSLinkedLists"),             SR::RTShadow_LinkedLists, 1);
        dest._uniforms.BindShaderResource(Hash64("RTSTriangles"),               SR::RTShadow_Triangles, 1);

        ::Assets::RegisterAssetDependency(_validationCallback, dest._shader->GetDependencyValidation());
    }

    auto LightingResolveShaders::GetShader(const LightShaderType& type) -> const LightShader*
    {
        auto index = type.AsIndex();
        if (index < _shaders.size()) return &_shaders[index];
        return nullptr; 
    }

    LightingResolveShaders::LightingResolveShaders(const Desc& desc)
    {
        using namespace RenderCore;
        _validationCallback = std::make_shared<::Assets::DependencyValidation>();
        _shaders.resize(LightShaderType::ReservedIndexCount());

            // find every sensible configuration, and build a new shader
            // and bound uniforms
        BuildShader(desc, LightShaderType(Directional, NoShadows, 0, 0));
        BuildShader(desc, LightShaderType(Directional, NoShadows, 1, 0));
        BuildShader(desc, LightShaderType(Directional, PerspectiveShadows, 0, 0));
        BuildShader(desc, LightShaderType(Directional, PerspectiveShadows, 1, 0));
        BuildShader(desc, LightShaderType(Directional, PerspectiveShadows, 0, 1));
        BuildShader(desc, LightShaderType(Directional, PerspectiveShadows, 1, 1));
        BuildShader(desc, LightShaderType(Directional, OrthShadows, 0, 0));
        BuildShader(desc, LightShaderType(Directional, OrthShadows, 1, 0));
        BuildShader(desc, LightShaderType(Directional, OrthShadows, 0, 1));
        BuildShader(desc, LightShaderType(Directional, OrthShadows, 1, 1));
        BuildShader(desc, LightShaderType(Directional, OrthHybridShadows, 0, 0));
        BuildShader(desc, LightShaderType(Directional, OrthHybridShadows, 1, 0));
        BuildShader(desc, LightShaderType(Directional, OrthHybridShadows, 0, 1));
        BuildShader(desc, LightShaderType(Directional, OrthHybridShadows, 1, 1));
        BuildShader(desc, LightShaderType(Point, NoShadows, 0, 0));
        BuildShader(desc, LightShaderType(Point, NoShadows, 1, 0));
        BuildShader(desc, LightShaderType(Point, PerspectiveShadows, 0, 0));
        BuildShader(desc, LightShaderType(Point, PerspectiveShadows, 1, 0));
        BuildShader(desc, LightShaderType(Point, PerspectiveShadows, 0, 1));
        BuildShader(desc, LightShaderType(Point, PerspectiveShadows, 1, 1));
    }

    LightingResolveShaders::~LightingResolveShaders() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////

    AmbientResolveShaders::AmbientResolveShaders(const Desc& desc)
    {
        using namespace RenderCore;
        StringMeld<256> definesTable;

        definesTable 
            << "GBUFFER_TYPE=" << desc._gbufferType
            << ";MSAA_SAMPLES=" << ((desc._msaaSampleCount<=1)?0:desc._msaaSampleCount)
            << ";SKY_PROJECTION=" << desc._skyProjectionType
            << ";CALCULATE_AMBIENT_OCCLUSION=" << desc._hasAO
            << ";CALCULATE_TILED_LIGHTS=" << desc._hasTiledLighting
            << ";CALCULATE_SCREENSPACE_REFLECTIONS=" << desc._hasSRR
            << ";RESOLVE_RANGE_FOG=" << desc._rangeFog
            ;

        if (desc._msaaSamplers) {
            definesTable << ";MSAA_SAMPLERS=1";
        }

        const char* vertexShader_viewFrustumVector = 
            desc._flipDirection
                ? "game/xleres/basic2D.vsh:fullscreen_flip_viewfrustumvector:vs_*"
                : "game/xleres/basic2D.vsh:fullscreen_viewfrustumvector:vs_*"
                ;

        auto* ambientLight = &::Assets::GetAssetDep<Metal::ShaderProgram>(
            vertexShader_viewFrustumVector, 
            "game/xleres/deferred/resolveambient.psh:ResolveAmbient:ps_*",
            definesTable.get());

        auto ambientLightUniforms = std::make_unique<Metal::BoundUniforms>(std::ref(*ambientLight));
        Techniques::TechniqueContext::BindGlobalUniforms(*ambientLightUniforms);
        ambientLightUniforms->BindConstantBuffer(Hash64("AmbientLightBuffer"), 0, 1);

        auto validationCallback = std::make_shared<::Assets::DependencyValidation>();
        ::Assets::RegisterAssetDependency(validationCallback, ambientLight->GetDependencyValidation());

        _ambientLight = std::move(ambientLight);
        _ambientLightUniforms = std::move(ambientLightUniforms);
        _validationCallback = std::move(validationCallback);
    }

    AmbientResolveShaders::~AmbientResolveShaders() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////


    #if defined(_DEBUG)
        void SaveGBuffer(RenderCore::Metal::DeviceContext* context, MainTargetsBox& mainTargets)
        {
            #if 0
                using namespace BufferUploads;
                BufferDesc stagingDesc[3];
                for (unsigned c=0; c<3; ++c) {
                    stagingDesc[c]._type = BufferDesc::Type::Texture;
                    stagingDesc[c]._bindFlags = 0;
                    stagingDesc[c]._cpuAccess = CPUAccess::Read;
                    stagingDesc[c]._gpuAccess = 0;
                    stagingDesc[c]._allocationRules = 0;
                    stagingDesc[c]._textureDesc = BufferUploads::TextureDesc::Plain2D(
                        mainTargets._desc._width, mainTargets._desc._height,
                        mainTargets._desc._gbufferFormats[c]._shaderReadFormat, 1, 0, 
                        mainTargets._desc._sampling);
                }

                const char* outputNames[] = { "gbuffer_diffuse.dds", "gbuffer_normals.dds", "gbuffer_parameters.dds" };
                auto& bufferUploads = *GetBufferUploads();
                for (unsigned c=0; c<3; ++c) {
                    if (mainTargets._gbufferTextures[c]) {
                        auto stagingTexture = bufferUploads.Transaction_Immediate(stagingDesc[c])->AdoptUnderlying();
                        context->GetUnderlying()->CopyResource(stagingTexture.get(), mainTargets._gbufferTextures[c].get());
                        D3DX11SaveTextureToFile(context->GetUnderlying(), stagingTexture.get(), D3DX11_IFF_DDS, outputNames[c]);
                    }
                }
            #endif
        }
    #endif


    void Deferred_DrawDebugging(RenderCore::Metal::DeviceContext* context, LightingParserContext& parserContext, MainTargetsBox& mainTargets)
    {
        using namespace RenderCore;
        TRY {
            context->BindPS(MakeResourceList(5, mainTargets._gbufferRTVsSRV[0], mainTargets._gbufferRTVsSRV[1], mainTargets._gbufferRTVsSRV[2], mainTargets._msaaDepthBufferSRV));
            const bool useMsaaSamplers = mainTargets._desc._sampling._sampleCount > 1;
            auto& debuggingShader = ::Assets::GetAssetDep<Metal::ShaderProgram>(
                "game/xleres/basic2D.vsh:fullscreen:vs_*", 
                "game/xleres/deferred/debugging.psh:GBufferDebugging:ps_*",
                useMsaaSamplers?"MSAA_SAMPLERS=1":"");
            context->Bind(debuggingShader);
            context->Bind(Techniques::CommonResources()._blendStraightAlpha);
            SetupVertexGeneratorShader(context);
            context->Draw(4);
        } 
        CATCH(const ::Assets::Exceptions::InvalidAsset& e) { parserContext.Process(e); }
        CATCH(const ::Assets::Exceptions::PendingAsset& e) { parserContext.Process(e); }
        CATCH_END

        context->UnbindPS<RenderCore::Metal::ShaderResourceView>(5, 4);
    }

}


