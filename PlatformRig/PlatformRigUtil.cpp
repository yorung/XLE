// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "PlatformRigUtil.h"
#include "../RenderCore/IDevice.h"
#include "../SceneEngine/LightDesc.h"
#include "../SceneEngine/LightingParserContext.h"   // just for ProjectionDesc
#include "../RenderCore/Techniques/TechniqueUtils.h"
#include "../ConsoleRig/Console.h"
#include "../ConsoleRig/IncludeLUA.h"
#include "../Utility/ParameterBox.h"
#include "../Utility/BitUtils.h"
#include "../Math/Transformations.h"
#include "../Math/ProjectionMath.h"

#include "../RenderCore/DX11/Metal/IncludeDX11.h"

namespace PlatformRig
{

    void GlobalTechniqueContext::SetInteger(const char name[], uint32 value)
    {
        _globalEnvironmentState.SetParameter((const utf8*)name, value);
    }

    GlobalTechniqueContext::GlobalTechniqueContext()
    {
        using namespace luabridge;
        auto* luaState = ConsoleRig::Console::GetInstance().GetLuaState();
        getGlobalNamespace(luaState)
            .beginClass<GlobalTechniqueContext>("TechniqueContext")
                .addFunction("SetI", &GlobalTechniqueContext::SetInteger)
            .endClass();
            
        setGlobal(luaState, this, "TechContext");
    }

    GlobalTechniqueContext::~GlobalTechniqueContext()
    {
        auto* luaState = ConsoleRig::Console::GetInstance().GetLuaState();
        lua_pushnil(luaState);
        lua_setglobal(luaState, "TechContext");
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    void ResizePresentationChain::OnResize(unsigned newWidth, unsigned newHeight)
    {
        if (_presentationChain) {
                //  When we become an icon, we'll end up with zero width and height.
                //  We can't actually resize the presentation to zero. And we can't
                //  delete the presentation chain from here. So maybe just do nothing.
            if (newWidth && newHeight) {
                _presentationChain->Resize(newWidth, newHeight);
            }
        }
    }

    ResizePresentationChain::ResizePresentationChain(std::shared_ptr<RenderCore::IPresentationChain> presentationChain)
    : _presentationChain(presentationChain)
    {}

///////////////////////////////////////////////////////////////////////////////////////////////////

    DefaultShadowFrustumSettings::DefaultShadowFrustumSettings()
    {
        const unsigned frustumCount = 5;
        const float maxDistanceFromCamera = 500.f;        // need really large distance because some models have a 100.f scale factor!
        const float frustumSizeFactor = 3.8f;
        const float focusDistance = 3.f;

        _frustumCount = frustumCount;
        _maxDistanceFromCamera = maxDistanceFromCamera;
        _frustumSizeFactor = frustumSizeFactor;
        _focusDistance = focusDistance;
        _flags = Flags::HighPrecisionDepths;
        _textureSize = 2048;

        _shadowSlopeScaledBias = Tweakable("ShadowSlopeScaledBias", 1.f);
        _shadowDepthBiasClamp = Tweakable("ShadowDepthBiasClamp", 0.f);
        _shadowRasterDepthBias = Tweakable("ShadowRasterDepthBias", 600);

        _worldSpaceResolveBias = 0.f;   // (-.3f)
        _tanBlurAngle = 0.00436f;
        _minBlurSearch = 0.5f;
        _maxBlurSearch = 25.f;
    }

}

#include "../Utility/Meta/ClassAccessors.h"
#include "../Utility/Meta/ClassAccessorsImpl.h"

template<> const ClassAccessors& GetAccessors<PlatformRig::DefaultShadowFrustumSettings>()
{
    using Obj = PlatformRig::DefaultShadowFrustumSettings;
    static ClassAccessors props(typeid(Obj).hash_code());
    static bool init = false;
    if (!init) {
        props.Add(u("FrustumCount"), DefaultGet(Obj, _frustumCount),  
            [](Obj& obj, unsigned value) { obj._frustumCount = Clamp(value, 1u, SceneEngine::MaxShadowTexturesPerLight); });
        props.Add(u("MaxDistanceFromCamera"),  DefaultGet(Obj, _maxDistanceFromCamera),   DefaultSet(Obj, _maxDistanceFromCamera));
        props.Add(u("FrustumSizeFactor"),   DefaultGet(Obj, _frustumSizeFactor),    DefaultSet(Obj, _frustumSizeFactor));
        props.Add(u("FocusDistance"),   DefaultGet(Obj, _focusDistance),    DefaultSet(Obj, _focusDistance));
        props.Add(u("Flags"),   DefaultGet(Obj, _flags),    DefaultSet(Obj, _flags));
        props.Add(u("TextureSize"),   DefaultGet(Obj, _textureSize),    
            [](Obj& obj, unsigned value) { obj._textureSize = 1<<(IntegerLog2(value-1)+1); });  // ceil to a power of two
        props.Add(u("ShadowSlopeScaledBias"),   DefaultGet(Obj, _shadowSlopeScaledBias),    DefaultSet(Obj, _shadowSlopeScaledBias));
        props.Add(u("ShadowDepthBiasClamp"),   DefaultGet(Obj, _shadowDepthBiasClamp),    DefaultSet(Obj, _shadowDepthBiasClamp));
        props.Add(u("ShadowRasterDepthBias"),   DefaultGet(Obj, _shadowRasterDepthBias),    DefaultSet(Obj, _shadowRasterDepthBias));
        props.Add(u("WorldSpaceResolveBias"),   DefaultGet(Obj, _worldSpaceResolveBias),    DefaultSet(Obj, _worldSpaceResolveBias));
        props.Add(u("BlurAngleDegrees"),   
            [](const Obj& obj) { return Rad2Deg(XlATan(obj._tanBlurAngle)); },
            [](Obj& obj, float value) { obj._tanBlurAngle = XlTan(Deg2Rad(value)); } );
        props.Add(u("MinBlurSearch"),   DefaultGet(Obj, _minBlurSearch),    DefaultSet(Obj, _minBlurSearch));
        props.Add(u("MaxBlurSearch"),   DefaultGet(Obj, _maxBlurSearch),    DefaultSet(Obj, _maxBlurSearch));
        init = true;
    }
    return props;
}

namespace PlatformRig
{

    static Float4x4 MakeWorldToLight(
        const Float3& negativeLightDirection,
        const Float3& position)
    {
        return InvertOrthonormalTransform(
            MakeCameraToWorld(-negativeLightDirection, Float3(1.f, 0.f, 0.f), position));
    }

    static std::pair<SceneEngine::ShadowProjectionDesc::Projections, Float4x4> 
        BuildBasicShadowProjections(
            const SceneEngine::LightDesc& lightDesc,
            const RenderCore::Techniques::ProjectionDesc& mainSceneProjectionDesc,
            const DefaultShadowFrustumSettings& settings)
    {
        using namespace SceneEngine;
        ShadowProjectionDesc::Projections result;

        const float shadowNearPlane = 1.f;
        const float shadowFarPlane = settings._maxDistanceFromCamera;
        static float shadowWidthScale = 3.f;
        static float projectionSizePower = 3.75f;
        float shadowProjectionDist = shadowFarPlane - shadowNearPlane;

        auto negativeLightDirection = lightDesc._negativeLightDirection;

        auto cameraPos = ExtractTranslation(mainSceneProjectionDesc._cameraToWorld);
        auto cameraForward = ExtractForward_Cam(mainSceneProjectionDesc._cameraToWorld);

            //  Calculate a simple set of shadow frustums.
            //  This method is non-ideal, but it's just a place holder for now
        result._count = 5;
        result._mode = ShadowProjectionDesc::Projections::Mode::Arbitrary;
        for (unsigned c=0; c<result._count; ++c) {
            const float projectionWidth = shadowWidthScale * std::pow(projectionSizePower, float(c));
            auto& p = result._fullProj[c];

            Float3 shiftDirection = cameraForward - negativeLightDirection * Dot(cameraForward, negativeLightDirection);

            Float3 focusPoint = cameraPos + (projectionWidth * 0.45f) * shiftDirection;
            auto lightViewMatrix = MakeWorldToLight(
                negativeLightDirection, focusPoint + (.5f * shadowProjectionDist) * negativeLightDirection);
                

                //  Note that the "flip" on the projection matrix is important here
                //  we need to make sure the correct faces are back-faced culled. If
                //  the wrong faces are culled, the results will still look close to being
                //  correct in many places, but there will be light leakage
            p._projectionMatrix = RenderCore::Techniques::OrthogonalProjection(
                -.5f * projectionWidth,  .5f * projectionWidth,
                 .5f * projectionWidth, -.5f * projectionWidth,
                shadowNearPlane, shadowFarPlane,
                RenderCore::Techniques::GeometricCoordinateSpace::RightHanded,
                RenderCore::Techniques::GetDefaultClipSpaceType());
            p._viewMatrix = lightViewMatrix;

            result._minimalProjection[c] = ExtractMinimalProjection(p._projectionMatrix);
        }
        
            //  Setup a single world-to-clip that contains all frustums within it. This will 
            //  be used for cull objects out of shadow casting.
        auto& lastProj = result._fullProj[result._count-1];
        auto worldToClip = Combine(lastProj._viewMatrix, lastProj._projectionMatrix);

        return std::make_pair(result, worldToClip);
    }

    static void CalculateCameraFrustumCornersDirection(
        Float3 result[4],
        const RenderCore::Techniques::ProjectionDesc& projDesc)
    {
        // For the given camera, calculate 4 vectors that represent the
        // the direction from the camera position to the frustum corners
        // (there are 8 frustum corners, but the directions to the far plane corners
        // are the same as the near plane corners)
        Float4x4 projection = projDesc._cameraToProjection;
        Float4x4 noTransCameraToWorld = projDesc._cameraToWorld;
        SetTranslation(noTransCameraToWorld, Float3(0.f, 0.f, 0.f));
        auto trans = Combine(InvertOrthonormalTransform(noTransCameraToWorld), projection);
        Float3 corners[8];
        CalculateAbsFrustumCorners(corners, trans);
        for (unsigned c=0; c<4; ++c) {
            result[c] = Normalize(corners[4+c]);    // use the more distance corners, on the far clip plane
        }
    }

    static std::pair<SceneEngine::ShadowProjectionDesc::Projections, Float4x4>  
        BuildSimpleOrthogonalShadowProjections(
            const SceneEngine::LightDesc& lightDesc,
            const RenderCore::Techniques::ProjectionDesc& mainSceneProjectionDesc,
            const DefaultShadowFrustumSettings& settings)
    {
        // We're going to build some basic adaptive shadow frustums. These frustums
        // all fit within the same "definition" orthogonal space. This means that
        // cascades can't be rotated or skewed relative to each other. Usually this 
        // should be fine, (and perhaps might reduce some flickering around the 
        // cascade edges) but it means that the cascades might not be as tightly
        // bound as they might be.

        using namespace SceneEngine;
        using namespace RenderCore;

        ShadowProjectionDesc::Projections result;
        result._count = settings._frustumCount;
        result._mode = ShadowProjectionDesc::Projections::Mode::Ortho;

        const float shadowNearPlane = -settings._maxDistanceFromCamera;
        const float shadowFarPlane = settings._maxDistanceFromCamera;

        float t = 0;
        for (unsigned c=0; c<result._count; ++c) { t += std::pow(settings._frustumSizeFactor, float(c)); }

        Float3 cameraPos = ExtractTranslation(mainSceneProjectionDesc._cameraToWorld);
        Float3 focusPoint = cameraPos + settings._focusDistance * ExtractForward(mainSceneProjectionDesc._cameraToWorld);
        result._definitionViewMatrix = MakeWorldToLight(lightDesc._negativeLightDirection, focusPoint);
        Float4x4 worldToLightProj = result._definitionViewMatrix;

            //  Calculate 4 vectors for the directions of the frustum corners, 
            //  relative to the camera position.
        Float3 frustumCornerDir[4];
        CalculateCameraFrustumCornersDirection(frustumCornerDir, mainSceneProjectionDesc);

        Float3 allCascadesMins( FLT_MAX,  FLT_MAX,  FLT_MAX);
		Float3 allCascadesMaxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		float distanceFromCamera = 0.f;
		for (unsigned f=0; f<result._count; ++f) {

			float camNearPlane = distanceFromCamera;
			distanceFromCamera += std::pow(settings._frustumSizeFactor, float(f)) * settings._maxDistanceFromCamera / t;
			float camFarPlane = distanceFromCamera;

                //  Find the frustum corners for this part of the camera frustum,
				//  and then build a shadow frustum that will contain those corners.
				//  Potentially not all of the camera frustum is full of geometry --
				//  if we knew which parts were full, and which were empty, we could
				//  optimise the shadow frustum further.

			Float3 absFrustumCorners[8];
			for (unsigned c = 0; c < 4; ++c) {
				absFrustumCorners[c] = cameraPos + camNearPlane * frustumCornerDir[c];
				absFrustumCorners[4 + c] = cameraPos + camFarPlane * frustumCornerDir[c];
            }

				//	Let's assume that we're not going to rotate the shadow frustum
				//	during this fitting. Then, this is easy... The shadow projection
				//	is orthogonal, so we just need to find the AABB in shadow-view space
				//	for these corners, and the projection parameters will match those very
				//	closely.
                //
                //  Note that we could potentially get a better result if we rotate the
                //  shadow frustum projection to better fit around the projected camera.
                //  It might make shadow texels creep and flicker as the projection changes,
                //  but perhaps a better implementation of this function could try that out.

			Float3 shadowViewSpace[8];
			Float3 shadowViewMins( FLT_MAX,  FLT_MAX,  FLT_MAX);
			Float3 shadowViewMaxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (unsigned c = 0; c < 8; c++) {
				shadowViewSpace[c] = TransformPoint(worldToLightProj, absFrustumCorners[c]);

					//	In our right handed coordinate space, the z coordinate in view space should
					//	be negative. But we always specify near & far in positive values. So
					//	we have to swap the sign of z here

				shadowViewSpace[c][2] = -shadowViewSpace[c][2];

				shadowViewMins[0] = std::min(shadowViewMins[0], shadowViewSpace[c][0]);
				shadowViewMins[1] = std::min(shadowViewMins[1], shadowViewSpace[c][1]);
				shadowViewMins[2] = std::min(shadowViewMins[2], shadowViewSpace[c][2]);
				shadowViewMaxs[0] = std::max(shadowViewMaxs[0], shadowViewSpace[c][0]);
				shadowViewMaxs[1] = std::max(shadowViewMaxs[1], shadowViewSpace[c][1]);
				shadowViewMaxs[2] = std::max(shadowViewMaxs[2], shadowViewSpace[c][2]);
			}

                //	We have to pull the min depth distance back towards the light
				//	This is so we can capture geometry that is between the light
				//	and the frustum

            shadowViewMins[2] = shadowNearPlane;
            shadowViewMaxs[2] = shadowFarPlane;

            result._orthoSub[f]._projMins = shadowViewMins;
            result._orthoSub[f]._projMaxs = shadowViewMaxs;

            allCascadesMins[0] = std::min(allCascadesMins[0], shadowViewMins[0]);
            allCascadesMins[1] = std::min(allCascadesMins[1], shadowViewMins[1]);
            allCascadesMins[2] = std::min(allCascadesMins[2], shadowViewMins[2]);
            allCascadesMaxs[0] = std::max(allCascadesMaxs[0], shadowViewMaxs[0]);
            allCascadesMaxs[1] = std::max(allCascadesMaxs[1], shadowViewMaxs[1]);
            allCascadesMaxs[2] = std::max(allCascadesMaxs[2], shadowViewMaxs[2]);
        }

        for (unsigned f=0; f<result._count; ++f) {
            result._fullProj[f]._viewMatrix = result._definitionViewMatrix;

            const auto& mins = result._orthoSub[f]._projMins;
            const auto& maxs = result._orthoSub[f]._projMaxs;
            Float4x4 projMatrix = Techniques::OrthogonalProjection(
                mins[0], mins[1], maxs[0], maxs[1], mins[2], maxs[2],
                Techniques::GeometricCoordinateSpace::RightHanded, Techniques::GetDefaultClipSpaceType());
            result._fullProj[f]._projectionMatrix = projMatrix;

            result._minimalProjection[f] = ExtractMinimalProjection(projMatrix);
        }

            //  When building the world to clip matrix, we want some to use some projection
            //  that projection that will contain all of the shadow frustums.
            //  We can use allCascadesMins and allCascadesMaxs to find the area of the 
            //  orthogonal space that is actually used. We just have to incorporate these
            //  mins and maxs into the projection matrix

        Float4x4 clippingProjMatrix = Techniques::OrthogonalProjection(
            allCascadesMins[0], allCascadesMins[1], allCascadesMaxs[0], allCascadesMaxs[1], 
            shadowNearPlane, shadowFarPlane,
            Techniques::GeometricCoordinateSpace::RightHanded, Techniques::GetDefaultClipSpaceType());

        Float4x4 worldToClip = Combine(result._definitionViewMatrix, clippingProjMatrix);
        return std::make_pair(result, worldToClip);
    }

    SceneEngine::ShadowProjectionDesc 
        CalculateDefaultShadowCascades(
            const SceneEngine::LightDesc& lightDesc,
            unsigned lightId,
            const RenderCore::Techniques::ProjectionDesc& mainSceneProjectionDesc,
            const DefaultShadowFrustumSettings& settings)
    {
            //  Build a default shadow frustum projection from the given inputs
            //  Note -- this is a very primitive implementation!
            //          But it actually works ok.
            //          Still, it's just a placeholder.

        using namespace SceneEngine;

        ShadowProjectionDesc result;
        result._width   = settings._textureSize;
        result._height  = settings._textureSize;
        if (settings._flags & DefaultShadowFrustumSettings::Flags::HighPrecisionDepths) {
            result._typelessFormat  = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_R24G8_TYPELESS;
            result._writeFormat     = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_D24_UNORM_S8_UINT;
            result._readFormat      = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        } else {
            result._typelessFormat  = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_R16_TYPELESS;
            result._writeFormat     = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_D16_UNORM;
            result._readFormat      = (RenderCore::Metal::NativeFormat::Enum)DXGI_FORMAT_R16_UNORM;
        }
        
        if (settings._flags & DefaultShadowFrustumSettings::Flags::ArbitraryCascades) {
            auto t = BuildBasicShadowProjections(lightDesc, mainSceneProjectionDesc, settings);
            result._projections = t.first;
            result._worldToClip = t.second;
        } else {
            auto t = BuildSimpleOrthogonalShadowProjections(lightDesc, mainSceneProjectionDesc, settings);
            result._projections = t.first;
            result._worldToClip = t.second;
        }

        if (settings._flags & DefaultShadowFrustumSettings::Flags::RayTraced) {
            result._resolveType = ShadowProjectionDesc::ResolveType::RayTraced;
        } else {
            result._resolveType = ShadowProjectionDesc::ResolveType::DepthTexture;
        }

        result._shadowSlopeScaledBias = settings._shadowSlopeScaledBias;
        result._shadowDepthBiasClamp = settings._shadowDepthBiasClamp;
        result._shadowRasterDepthBias = settings._shadowRasterDepthBias;

        result._worldSpaceResolveBias = settings._worldSpaceResolveBias;
        result._tanBlurAngle = settings._tanBlurAngle;
        result._minBlurSearch = settings._minBlurSearch;
        result._maxBlurSearch = settings._maxBlurSearch;

        result._lightId = lightId;

        return result;
    }

}


