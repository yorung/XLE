// -------------------------------------------------------------------------------------------------------------------
// Generated code, do not edit
// Command Line:  DomGen "xleroot.xsd" "Schema.cs" "gap" "LevelEditorXLE"
// -------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;

using Sce.Atf.Dom;

namespace LevelEditorXLE
{
    public static class Schema
    {
        public const string NS = "gap";

        public static void Initialize(XmlSchemaTypeCollection typeCollection)
        {
            Initialize((ns,name)=>typeCollection.GetNodeType(ns,name),
                (ns,name)=>typeCollection.GetRootElement(ns,name));
        }

        public static void Initialize(IDictionary<string, XmlSchemaTypeCollection> typeCollections)
        {
            Initialize((ns,name)=>typeCollections[ns].GetNodeType(name),
                (ns,name)=>typeCollections[ns].GetRootElement(name));
        }

        private static void Initialize(Func<string, string, DomNodeType> getNodeType, Func<string, string, ChildInfo> getRootElement)
        {
            placementsDocumentType.Type = getNodeType("gap", "placementsDocumentType");
            placementsDocumentType.nameAttribute = placementsDocumentType.Type.GetAttributeInfo("name");
            placementsDocumentType.placementChild = placementsDocumentType.Type.GetChildInfo("placement");

            abstractPlacementObjectType.Type = getNodeType("gap", "abstractPlacementObjectType");
            abstractPlacementObjectType.transformAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("transform");
            abstractPlacementObjectType.translateAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("translate");
            abstractPlacementObjectType.rotateAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("rotate");
            abstractPlacementObjectType.scaleAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("scale");
            abstractPlacementObjectType.pivotAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("pivot");
            abstractPlacementObjectType.transformationTypeAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("transformationType");
            abstractPlacementObjectType.visibleAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("visible");
            abstractPlacementObjectType.lockedAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("locked");
            abstractPlacementObjectType.IDAttribute = abstractPlacementObjectType.Type.GetAttributeInfo("ID");

            transformObjectType.Type = getNodeType("gap", "transformObjectType");
            transformObjectType.transformAttribute = transformObjectType.Type.GetAttributeInfo("transform");
            transformObjectType.translateAttribute = transformObjectType.Type.GetAttributeInfo("translate");
            transformObjectType.rotateAttribute = transformObjectType.Type.GetAttributeInfo("rotate");
            transformObjectType.scaleAttribute = transformObjectType.Type.GetAttributeInfo("scale");
            transformObjectType.pivotAttribute = transformObjectType.Type.GetAttributeInfo("pivot");
            transformObjectType.transformationTypeAttribute = transformObjectType.Type.GetAttributeInfo("transformationType");

            gameType.Type = getNodeType("gap", "gameType");
            gameType.nameAttribute = gameType.Type.GetAttributeInfo("name");
            gameType.fogEnabledAttribute = gameType.Type.GetAttributeInfo("fogEnabled");
            gameType.fogColorAttribute = gameType.Type.GetAttributeInfo("fogColor");
            gameType.fogRangeAttribute = gameType.Type.GetAttributeInfo("fogRange");
            gameType.fogDensityAttribute = gameType.Type.GetAttributeInfo("fogDensity");
            gameType.gameObjectFolderChild = gameType.Type.GetChildInfo("gameObjectFolder");
            gameType.layersChild = gameType.Type.GetChildInfo("layers");
            gameType.bookmarksChild = gameType.Type.GetChildInfo("bookmarks");
            gameType.gameReferenceChild = gameType.Type.GetChildInfo("gameReference");
            gameType.gridChild = gameType.Type.GetChildInfo("grid");

            gameObjectFolderType.Type = getNodeType("gap", "gameObjectFolderType");
            gameObjectFolderType.nameAttribute = gameObjectFolderType.Type.GetAttributeInfo("name");
            gameObjectFolderType.visibleAttribute = gameObjectFolderType.Type.GetAttributeInfo("visible");
            gameObjectFolderType.lockedAttribute = gameObjectFolderType.Type.GetAttributeInfo("locked");
            gameObjectFolderType.gameObjectChild = gameObjectFolderType.Type.GetChildInfo("gameObject");
            gameObjectFolderType.folderChild = gameObjectFolderType.Type.GetChildInfo("folder");

            gameObjectType.Type = getNodeType("gap", "gameObjectType");
            gameObjectType.transformAttribute = gameObjectType.Type.GetAttributeInfo("transform");
            gameObjectType.translateAttribute = gameObjectType.Type.GetAttributeInfo("translate");
            gameObjectType.rotateAttribute = gameObjectType.Type.GetAttributeInfo("rotate");
            gameObjectType.scaleAttribute = gameObjectType.Type.GetAttributeInfo("scale");
            gameObjectType.pivotAttribute = gameObjectType.Type.GetAttributeInfo("pivot");
            gameObjectType.transformationTypeAttribute = gameObjectType.Type.GetAttributeInfo("transformationType");
            gameObjectType.nameAttribute = gameObjectType.Type.GetAttributeInfo("name");
            gameObjectType.visibleAttribute = gameObjectType.Type.GetAttributeInfo("visible");
            gameObjectType.lockedAttribute = gameObjectType.Type.GetAttributeInfo("locked");

            layersType.Type = getNodeType("gap", "layersType");
            layersType.layerChild = layersType.Type.GetChildInfo("layer");

            layerType.Type = getNodeType("gap", "layerType");
            layerType.nameAttribute = layerType.Type.GetAttributeInfo("name");
            layerType.gameObjectReferenceChild = layerType.Type.GetChildInfo("gameObjectReference");
            layerType.layerChild = layerType.Type.GetChildInfo("layer");

            gameObjectReferenceType.Type = getNodeType("gap", "gameObjectReferenceType");
            gameObjectReferenceType.refAttribute = gameObjectReferenceType.Type.GetAttributeInfo("ref");

            bookmarksType.Type = getNodeType("gap", "bookmarksType");
            bookmarksType.bookmarkChild = bookmarksType.Type.GetChildInfo("bookmark");

            bookmarkType.Type = getNodeType("gap", "bookmarkType");
            bookmarkType.nameAttribute = bookmarkType.Type.GetAttributeInfo("name");
            bookmarkType.cameraChild = bookmarkType.Type.GetChildInfo("camera");
            bookmarkType.bookmarkChild = bookmarkType.Type.GetChildInfo("bookmark");

            cameraType.Type = getNodeType("gap", "cameraType");
            cameraType.eyeAttribute = cameraType.Type.GetAttributeInfo("eye");
            cameraType.lookAtPointAttribute = cameraType.Type.GetAttributeInfo("lookAtPoint");
            cameraType.upVectorAttribute = cameraType.Type.GetAttributeInfo("upVector");
            cameraType.viewTypeAttribute = cameraType.Type.GetAttributeInfo("viewType");
            cameraType.yFovAttribute = cameraType.Type.GetAttributeInfo("yFov");
            cameraType.nearZAttribute = cameraType.Type.GetAttributeInfo("nearZ");
            cameraType.farZAttribute = cameraType.Type.GetAttributeInfo("farZ");
            cameraType.focusRadiusAttribute = cameraType.Type.GetAttributeInfo("focusRadius");

            gameReferenceType.Type = getNodeType("gap", "gameReferenceType");
            gameReferenceType.nameAttribute = gameReferenceType.Type.GetAttributeInfo("name");
            gameReferenceType.refAttribute = gameReferenceType.Type.GetAttributeInfo("ref");
            gameReferenceType.tagsAttribute = gameReferenceType.Type.GetAttributeInfo("tags");

            gridType.Type = getNodeType("gap", "gridType");
            gridType.sizeAttribute = gridType.Type.GetAttributeInfo("size");
            gridType.subdivisionsAttribute = gridType.Type.GetAttributeInfo("subdivisions");
            gridType.heightAttribute = gridType.Type.GetAttributeInfo("height");
            gridType.snapAttribute = gridType.Type.GetAttributeInfo("snap");
            gridType.visibleAttribute = gridType.Type.GetAttributeInfo("visible");

            ambientSettingsType.Type = getNodeType("gap", "ambientSettingsType");
            ambientSettingsType.AmbientLightAttribute = ambientSettingsType.Type.GetAttributeInfo("AmbientLight");
            ambientSettingsType.AmbientBrightnessAttribute = ambientSettingsType.Type.GetAttributeInfo("AmbientBrightness");
            ambientSettingsType.SkyTextureAttribute = ambientSettingsType.Type.GetAttributeInfo("SkyTexture");
            ambientSettingsType.SkyReflectionScaleAttribute = ambientSettingsType.Type.GetAttributeInfo("SkyReflectionScale");
            ambientSettingsType.SkyReflectionBlurrinessAttribute = ambientSettingsType.Type.GetAttributeInfo("SkyReflectionBlurriness");
            ambientSettingsType.SkyBrightnessAttribute = ambientSettingsType.Type.GetAttributeInfo("SkyBrightness");
            ambientSettingsType.RangeFogInscatterAttribute = ambientSettingsType.Type.GetAttributeInfo("RangeFogInscatter");
            ambientSettingsType.RangeFogInscatterScaleAttribute = ambientSettingsType.Type.GetAttributeInfo("RangeFogInscatterScale");
            ambientSettingsType.RangeFogThicknessAttribute = ambientSettingsType.Type.GetAttributeInfo("RangeFogThickness");
            ambientSettingsType.RangeFogThicknessScaleAttribute = ambientSettingsType.Type.GetAttributeInfo("RangeFogThicknessScale");
            ambientSettingsType.AtmosBlurStdDevAttribute = ambientSettingsType.Type.GetAttributeInfo("AtmosBlurStdDev");
            ambientSettingsType.AtmosBlurStartAttribute = ambientSettingsType.Type.GetAttributeInfo("AtmosBlurStart");
            ambientSettingsType.AtmosBlurEndAttribute = ambientSettingsType.Type.GetAttributeInfo("AtmosBlurEnd");
            ambientSettingsType.FlagsAttribute = ambientSettingsType.Type.GetAttributeInfo("Flags");

            toneMapSettingsType.Type = getNodeType("gap", "toneMapSettingsType");
            toneMapSettingsType.BloomScaleAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomScale");
            toneMapSettingsType.BloomThresholdAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomThreshold");
            toneMapSettingsType.BloomRampingFactorAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomRampingFactor");
            toneMapSettingsType.BloomDesaturationFactorAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomDesaturationFactor");
            toneMapSettingsType.BloomBlurStdDevAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomBlurStdDev");
            toneMapSettingsType.BloomBrightnessAttribute = toneMapSettingsType.Type.GetAttributeInfo("BloomBrightness");
            toneMapSettingsType.SceneKeyAttribute = toneMapSettingsType.Type.GetAttributeInfo("SceneKey");
            toneMapSettingsType.LuminanceMinAttribute = toneMapSettingsType.Type.GetAttributeInfo("LuminanceMin");
            toneMapSettingsType.LuminanceMaxAttribute = toneMapSettingsType.Type.GetAttributeInfo("LuminanceMax");
            toneMapSettingsType.WhitePointAttribute = toneMapSettingsType.Type.GetAttributeInfo("WhitePoint");
            toneMapSettingsType.FlagsAttribute = toneMapSettingsType.Type.GetAttributeInfo("Flags");

            envObjectType.Type = getNodeType("gap", "envObjectType");
            envObjectType.transformAttribute = envObjectType.Type.GetAttributeInfo("transform");
            envObjectType.translateAttribute = envObjectType.Type.GetAttributeInfo("translate");
            envObjectType.rotateAttribute = envObjectType.Type.GetAttributeInfo("rotate");
            envObjectType.scaleAttribute = envObjectType.Type.GetAttributeInfo("scale");
            envObjectType.pivotAttribute = envObjectType.Type.GetAttributeInfo("pivot");
            envObjectType.transformationTypeAttribute = envObjectType.Type.GetAttributeInfo("transformationType");
            envObjectType.nameAttribute = envObjectType.Type.GetAttributeInfo("name");
            envObjectType.visibleAttribute = envObjectType.Type.GetAttributeInfo("visible");
            envObjectType.lockedAttribute = envObjectType.Type.GetAttributeInfo("locked");

            envMiscType.Type = getNodeType("gap", "envMiscType");

            envSettingsType.Type = getNodeType("gap", "envSettingsType");
            envSettingsType.NameAttribute = envSettingsType.Type.GetAttributeInfo("Name");
            envSettingsType.objectsChild = envSettingsType.Type.GetChildInfo("objects");
            envSettingsType.settingsChild = envSettingsType.Type.GetChildInfo("settings");
            envSettingsType.ambientChild = envSettingsType.Type.GetChildInfo("ambient");
            envSettingsType.tonemapChild = envSettingsType.Type.GetChildInfo("tonemap");

            envSettingsFolderType.Type = getNodeType("gap", "envSettingsFolderType");
            envSettingsFolderType.ExportTargetAttribute = envSettingsFolderType.Type.GetAttributeInfo("ExportTarget");
            envSettingsFolderType.ExportEnabledAttribute = envSettingsFolderType.Type.GetAttributeInfo("ExportEnabled");
            envSettingsFolderType.settingsChild = envSettingsFolderType.Type.GetChildInfo("settings");

            directionalLightType.Type = getNodeType("gap", "directionalLightType");
            directionalLightType.transformAttribute = directionalLightType.Type.GetAttributeInfo("transform");
            directionalLightType.translateAttribute = directionalLightType.Type.GetAttributeInfo("translate");
            directionalLightType.rotateAttribute = directionalLightType.Type.GetAttributeInfo("rotate");
            directionalLightType.scaleAttribute = directionalLightType.Type.GetAttributeInfo("scale");
            directionalLightType.pivotAttribute = directionalLightType.Type.GetAttributeInfo("pivot");
            directionalLightType.transformationTypeAttribute = directionalLightType.Type.GetAttributeInfo("transformationType");
            directionalLightType.nameAttribute = directionalLightType.Type.GetAttributeInfo("name");
            directionalLightType.visibleAttribute = directionalLightType.Type.GetAttributeInfo("visible");
            directionalLightType.lockedAttribute = directionalLightType.Type.GetAttributeInfo("locked");
            directionalLightType.DiffuseAttribute = directionalLightType.Type.GetAttributeInfo("Diffuse");
            directionalLightType.DiffuseBrightnessAttribute = directionalLightType.Type.GetAttributeInfo("DiffuseBrightness");
            directionalLightType.DiffuseModelAttribute = directionalLightType.Type.GetAttributeInfo("DiffuseModel");
            directionalLightType.DiffuseWideningMinAttribute = directionalLightType.Type.GetAttributeInfo("DiffuseWideningMin");
            directionalLightType.DiffuseWideningMaxAttribute = directionalLightType.Type.GetAttributeInfo("DiffuseWideningMax");
            directionalLightType.SpecularAttribute = directionalLightType.Type.GetAttributeInfo("Specular");
            directionalLightType.SpecularBrightnessAttribute = directionalLightType.Type.GetAttributeInfo("SpecularBrightness");
            directionalLightType.SpecularNonMetalBrightnessAttribute = directionalLightType.Type.GetAttributeInfo("SpecularNonMetalBrightness");
            directionalLightType.FlagsAttribute = directionalLightType.Type.GetAttributeInfo("Flags");
            directionalLightType.ShadowResolveModelAttribute = directionalLightType.Type.GetAttributeInfo("ShadowResolveModel");

            shadowFrustumSettings.Type = getNodeType("gap", "shadowFrustumSettings");
            shadowFrustumSettings.NameAttribute = shadowFrustumSettings.Type.GetAttributeInfo("Name");
            shadowFrustumSettings.FlagsAttribute = shadowFrustumSettings.Type.GetAttributeInfo("Flags");
            shadowFrustumSettings.FrustumCountAttribute = shadowFrustumSettings.Type.GetAttributeInfo("FrustumCount");
            shadowFrustumSettings.MaxDistanceFromCameraAttribute = shadowFrustumSettings.Type.GetAttributeInfo("MaxDistanceFromCamera");
            shadowFrustumSettings.FrustumSizeFactorAttribute = shadowFrustumSettings.Type.GetAttributeInfo("FrustumSizeFactor");
            shadowFrustumSettings.FocusDistanceAttribute = shadowFrustumSettings.Type.GetAttributeInfo("FocusDistance");
            shadowFrustumSettings.TextureSizeAttribute = shadowFrustumSettings.Type.GetAttributeInfo("TextureSize");
            shadowFrustumSettings.ShadowSlopeScaledBiasAttribute = shadowFrustumSettings.Type.GetAttributeInfo("ShadowSlopeScaledBias");
            shadowFrustumSettings.ShadowDepthBiasClampAttribute = shadowFrustumSettings.Type.GetAttributeInfo("ShadowDepthBiasClamp");
            shadowFrustumSettings.ShadowRasterDepthBiasAttribute = shadowFrustumSettings.Type.GetAttributeInfo("ShadowRasterDepthBias");
            shadowFrustumSettings.WorldSpaceResolveBiasAttribute = shadowFrustumSettings.Type.GetAttributeInfo("WorldSpaceResolveBias");
            shadowFrustumSettings.BlurAngleDegreesAttribute = shadowFrustumSettings.Type.GetAttributeInfo("BlurAngleDegrees");
            shadowFrustumSettings.MinBlurSearchAttribute = shadowFrustumSettings.Type.GetAttributeInfo("MinBlurSearch");
            shadowFrustumSettings.MaxBlurSearchAttribute = shadowFrustumSettings.Type.GetAttributeInfo("MaxBlurSearch");
            shadowFrustumSettings.LightAttribute = shadowFrustumSettings.Type.GetAttributeInfo("Light");

            oceanSettings.Type = getNodeType("gap", "oceanSettings");
            oceanSettings.EnableAttribute = oceanSettings.Type.GetAttributeInfo("Enable");
            oceanSettings.WindAngleAttribute = oceanSettings.Type.GetAttributeInfo("WindAngle");
            oceanSettings.WindVelocityAttribute = oceanSettings.Type.GetAttributeInfo("WindVelocity");
            oceanSettings.PhysicalDimensionsAttribute = oceanSettings.Type.GetAttributeInfo("PhysicalDimensions");
            oceanSettings.GridDimensionsAttribute = oceanSettings.Type.GetAttributeInfo("GridDimensions");
            oceanSettings.StrengthConstantXYAttribute = oceanSettings.Type.GetAttributeInfo("StrengthConstantXY");
            oceanSettings.StrengthConstantZAttribute = oceanSettings.Type.GetAttributeInfo("StrengthConstantZ");
            oceanSettings.DetailNormalsStrengthAttribute = oceanSettings.Type.GetAttributeInfo("DetailNormalsStrength");
            oceanSettings.SpectrumFadeAttribute = oceanSettings.Type.GetAttributeInfo("SpectrumFade");
            oceanSettings.ScaleAgainstWindAttribute = oceanSettings.Type.GetAttributeInfo("ScaleAgainstWind");
            oceanSettings.SuppressionFactorAttribute = oceanSettings.Type.GetAttributeInfo("SuppressionFactor");
            oceanSettings.GridShiftSpeedAttribute = oceanSettings.Type.GetAttributeInfo("GridShiftSpeed");
            oceanSettings.BaseHeightAttribute = oceanSettings.Type.GetAttributeInfo("BaseHeight");
            oceanSettings.FoamThresholdAttribute = oceanSettings.Type.GetAttributeInfo("FoamThreshold");
            oceanSettings.FoamIncreaseSpeedAttribute = oceanSettings.Type.GetAttributeInfo("FoamIncreaseSpeed");
            oceanSettings.FoamIncreaseClampAttribute = oceanSettings.Type.GetAttributeInfo("FoamIncreaseClamp");
            oceanSettings.FoamDecreaseAttribute = oceanSettings.Type.GetAttributeInfo("FoamDecrease");

            oceanLightingSettings.Type = getNodeType("gap", "oceanLightingSettings");
            oceanLightingSettings.FoamBrightnessAttribute = oceanLightingSettings.Type.GetAttributeInfo("FoamBrightness");
            oceanLightingSettings.OpticalThicknessScalarAttribute = oceanLightingSettings.Type.GetAttributeInfo("OpticalThicknessScalar");
            oceanLightingSettings.OpticalThicknessColorAttribute = oceanLightingSettings.Type.GetAttributeInfo("OpticalThicknessColor");
            oceanLightingSettings.SkyReflectionBrightnessAttribute = oceanLightingSettings.Type.GetAttributeInfo("SkyReflectionBrightness");
            oceanLightingSettings.UpwellingScaleAttribute = oceanLightingSettings.Type.GetAttributeInfo("UpwellingScale");
            oceanLightingSettings.RefractiveIndexAttribute = oceanLightingSettings.Type.GetAttributeInfo("RefractiveIndex");
            oceanLightingSettings.ReflectionBumpScaleAttribute = oceanLightingSettings.Type.GetAttributeInfo("ReflectionBumpScale");
            oceanLightingSettings.DetailNormalFrequencyAttribute = oceanLightingSettings.Type.GetAttributeInfo("DetailNormalFrequency");
            oceanLightingSettings.SpecularityFrequencyAttribute = oceanLightingSettings.Type.GetAttributeInfo("SpecularityFrequency");
            oceanLightingSettings.MatSpecularMinAttribute = oceanLightingSettings.Type.GetAttributeInfo("MatSpecularMin");
            oceanLightingSettings.MatSpecularMaxAttribute = oceanLightingSettings.Type.GetAttributeInfo("MatSpecularMax");
            oceanLightingSettings.MatRoughnessAttribute = oceanLightingSettings.Type.GetAttributeInfo("MatRoughness");

            envUtilityType.Type = getNodeType("gap", "envUtilityType");
            envUtilityType.SunAngleAttribute = envUtilityType.Type.GetAttributeInfo("SunAngle");
            envUtilityType.SunNameAttribute = envUtilityType.Type.GetAttributeInfo("SunName");

            fogVolumeType.Type = getNodeType("gap", "fogVolumeType");
            fogVolumeType.transformAttribute = fogVolumeType.Type.GetAttributeInfo("transform");
            fogVolumeType.translateAttribute = fogVolumeType.Type.GetAttributeInfo("translate");
            fogVolumeType.rotateAttribute = fogVolumeType.Type.GetAttributeInfo("rotate");
            fogVolumeType.scaleAttribute = fogVolumeType.Type.GetAttributeInfo("scale");
            fogVolumeType.pivotAttribute = fogVolumeType.Type.GetAttributeInfo("pivot");
            fogVolumeType.transformationTypeAttribute = fogVolumeType.Type.GetAttributeInfo("transformationType");
            fogVolumeType.nameAttribute = fogVolumeType.Type.GetAttributeInfo("name");
            fogVolumeType.visibleAttribute = fogVolumeType.Type.GetAttributeInfo("visible");
            fogVolumeType.lockedAttribute = fogVolumeType.Type.GetAttributeInfo("locked");
            fogVolumeType.DensityAttribute = fogVolumeType.Type.GetAttributeInfo("Density");
            fogVolumeType.NoiseDensityScaleAttribute = fogVolumeType.Type.GetAttributeInfo("NoiseDensityScale");
            fogVolumeType.NoiseSpeedAttribute = fogVolumeType.Type.GetAttributeInfo("NoiseSpeed");
            fogVolumeType.ForwardColorAttribute = fogVolumeType.Type.GetAttributeInfo("ForwardColor");
            fogVolumeType.ForwardColorScaleAttribute = fogVolumeType.Type.GetAttributeInfo("ForwardColorScale");
            fogVolumeType.BackColorAttribute = fogVolumeType.Type.GetAttributeInfo("BackColor");
            fogVolumeType.BackColorScaleAttribute = fogVolumeType.Type.GetAttributeInfo("BackColorScale");
            fogVolumeType.ESM_CAttribute = fogVolumeType.Type.GetAttributeInfo("ESM_C");
            fogVolumeType.ShadowBiasAttribute = fogVolumeType.Type.GetAttributeInfo("ShadowBias");
            fogVolumeType.JitteringAmountAttribute = fogVolumeType.Type.GetAttributeInfo("JitteringAmount");
            fogVolumeType.HeightStartAttribute = fogVolumeType.Type.GetAttributeInfo("HeightStart");
            fogVolumeType.HeightEndAttribute = fogVolumeType.Type.GetAttributeInfo("HeightEnd");

            fogVolumeRendererType.Type = getNodeType("gap", "fogVolumeRendererType");
            fogVolumeRendererType.BlurredShadowSizeAttribute = fogVolumeRendererType.Type.GetAttributeInfo("BlurredShadowSize");
            fogVolumeRendererType.ShadowDownsampleAttribute = fogVolumeRendererType.Type.GetAttributeInfo("ShadowDownsample");
            fogVolumeRendererType.SkipShadowFrustumsAttribute = fogVolumeRendererType.Type.GetAttributeInfo("SkipShadowFrustums");
            fogVolumeRendererType.MaxShadowFrustumsAttribute = fogVolumeRendererType.Type.GetAttributeInfo("MaxShadowFrustums");
            fogVolumeRendererType.GridDimensionsAttribute = fogVolumeRendererType.Type.GetAttributeInfo("GridDimensions");
            fogVolumeRendererType.WorldSpaceGridDepthAttribute = fogVolumeRendererType.Type.GetAttributeInfo("WorldSpaceGridDepth");

            shallowSurfaceType.Type = getNodeType("gap", "shallowSurfaceType");
            shallowSurfaceType.transformAttribute = shallowSurfaceType.Type.GetAttributeInfo("transform");
            shallowSurfaceType.translateAttribute = shallowSurfaceType.Type.GetAttributeInfo("translate");
            shallowSurfaceType.rotateAttribute = shallowSurfaceType.Type.GetAttributeInfo("rotate");
            shallowSurfaceType.scaleAttribute = shallowSurfaceType.Type.GetAttributeInfo("scale");
            shallowSurfaceType.pivotAttribute = shallowSurfaceType.Type.GetAttributeInfo("pivot");
            shallowSurfaceType.transformationTypeAttribute = shallowSurfaceType.Type.GetAttributeInfo("transformationType");
            shallowSurfaceType.nameAttribute = shallowSurfaceType.Type.GetAttributeInfo("name");
            shallowSurfaceType.visibleAttribute = shallowSurfaceType.Type.GetAttributeInfo("visible");
            shallowSurfaceType.lockedAttribute = shallowSurfaceType.Type.GetAttributeInfo("locked");
            shallowSurfaceType.MarkerAttribute = shallowSurfaceType.Type.GetAttributeInfo("Marker");
            shallowSurfaceType.GridPhysicalSizeAttribute = shallowSurfaceType.Type.GetAttributeInfo("GridPhysicalSize");
            shallowSurfaceType.GridDimsAttribute = shallowSurfaceType.Type.GetAttributeInfo("GridDims");
            shallowSurfaceType.SimGridCountAttribute = shallowSurfaceType.Type.GetAttributeInfo("SimGridCount");
            shallowSurfaceType.BaseHeightAttribute = shallowSurfaceType.Type.GetAttributeInfo("BaseHeight");
            shallowSurfaceType.SimMethodAttribute = shallowSurfaceType.Type.GetAttributeInfo("SimMethod");
            shallowSurfaceType.RainQuantityAttribute = shallowSurfaceType.Type.GetAttributeInfo("RainQuantity");
            shallowSurfaceType.EvaporationConstantAttribute = shallowSurfaceType.Type.GetAttributeInfo("EvaporationConstant");
            shallowSurfaceType.PressureConstantAttribute = shallowSurfaceType.Type.GetAttributeInfo("PressureConstant");
            shallowSurfaceType.OpticalThicknessColorAttribute = shallowSurfaceType.Type.GetAttributeInfo("OpticalThicknessColor");
            shallowSurfaceType.OpticalThicknessScalarAttribute = shallowSurfaceType.Type.GetAttributeInfo("OpticalThicknessScalar");
            shallowSurfaceType.FoamColorAttribute = shallowSurfaceType.Type.GetAttributeInfo("FoamColor");
            shallowSurfaceType.SpecularAttribute = shallowSurfaceType.Type.GetAttributeInfo("Specular");
            shallowSurfaceType.RoughnessAttribute = shallowSurfaceType.Type.GetAttributeInfo("Roughness");
            shallowSurfaceType.RefractiveIndexAttribute = shallowSurfaceType.Type.GetAttributeInfo("RefractiveIndex");
            shallowSurfaceType.UpwellingScaleAttribute = shallowSurfaceType.Type.GetAttributeInfo("UpwellingScale");
            shallowSurfaceType.SkyReflectionScaleAttribute = shallowSurfaceType.Type.GetAttributeInfo("SkyReflectionScale");

            placementsCellReferenceType.Type = getNodeType("gap", "placementsCellReferenceType");
            placementsCellReferenceType.refAttribute = placementsCellReferenceType.Type.GetAttributeInfo("ref");
            placementsCellReferenceType.ExportTargetAttribute = placementsCellReferenceType.Type.GetAttributeInfo("ExportTarget");
            placementsCellReferenceType.nameAttribute = placementsCellReferenceType.Type.GetAttributeInfo("name");
            placementsCellReferenceType.captureMinsAttribute = placementsCellReferenceType.Type.GetAttributeInfo("captureMins");
            placementsCellReferenceType.captureMaxsAttribute = placementsCellReferenceType.Type.GetAttributeInfo("captureMaxs");
            placementsCellReferenceType.offsetAttribute = placementsCellReferenceType.Type.GetAttributeInfo("offset");
            placementsCellReferenceType.ExportEnabledAttribute = placementsCellReferenceType.Type.GetAttributeInfo("ExportEnabled");
            placementsCellReferenceType.cachedCellMinsAttribute = placementsCellReferenceType.Type.GetAttributeInfo("cachedCellMins");
            placementsCellReferenceType.cachedCellMaxsAttribute = placementsCellReferenceType.Type.GetAttributeInfo("cachedCellMaxs");

            placementsFolderType.Type = getNodeType("gap", "placementsFolderType");
            placementsFolderType.baseEditorPathAttribute = placementsFolderType.Type.GetAttributeInfo("baseEditorPath");
            placementsFolderType.baseExportPathAttribute = placementsFolderType.Type.GetAttributeInfo("baseExportPath");
            placementsFolderType.CellCountAttribute = placementsFolderType.Type.GetAttributeInfo("CellCount");
            placementsFolderType.CellsOriginAttribute = placementsFolderType.Type.GetAttributeInfo("CellsOrigin");
            placementsFolderType.CellSizeAttribute = placementsFolderType.Type.GetAttributeInfo("CellSize");
            placementsFolderType.ExportTargetAttribute = placementsFolderType.Type.GetAttributeInfo("ExportTarget");
            placementsFolderType.ExportEnabledAttribute = placementsFolderType.Type.GetAttributeInfo("ExportEnabled");
            placementsFolderType.cellChild = placementsFolderType.Type.GetChildInfo("cell");

            placementObjectType.Type = getNodeType("gap", "placementObjectType");
            placementObjectType.transformAttribute = placementObjectType.Type.GetAttributeInfo("transform");
            placementObjectType.translateAttribute = placementObjectType.Type.GetAttributeInfo("translate");
            placementObjectType.rotateAttribute = placementObjectType.Type.GetAttributeInfo("rotate");
            placementObjectType.scaleAttribute = placementObjectType.Type.GetAttributeInfo("scale");
            placementObjectType.pivotAttribute = placementObjectType.Type.GetAttributeInfo("pivot");
            placementObjectType.transformationTypeAttribute = placementObjectType.Type.GetAttributeInfo("transformationType");
            placementObjectType.visibleAttribute = placementObjectType.Type.GetAttributeInfo("visible");
            placementObjectType.lockedAttribute = placementObjectType.Type.GetAttributeInfo("locked");
            placementObjectType.IDAttribute = placementObjectType.Type.GetAttributeInfo("ID");
            placementObjectType.modelAttribute = placementObjectType.Type.GetAttributeInfo("model");
            placementObjectType.materialAttribute = placementObjectType.Type.GetAttributeInfo("material");

            abstractTerrainMaterialDescType.Type = getNodeType("gap", "abstractTerrainMaterialDescType");
            abstractTerrainMaterialDescType.MaterialIdAttribute = abstractTerrainMaterialDescType.Type.GetAttributeInfo("MaterialId");

            terrainBaseTextureType.Type = getNodeType("gap", "terrainBaseTextureType");
            terrainBaseTextureType.diffusedimsAttribute = terrainBaseTextureType.Type.GetAttributeInfo("diffusedims");
            terrainBaseTextureType.normaldimsAttribute = terrainBaseTextureType.Type.GetAttributeInfo("normaldims");
            terrainBaseTextureType.paramdimsAttribute = terrainBaseTextureType.Type.GetAttributeInfo("paramdims");
            terrainBaseTextureType.materialChild = terrainBaseTextureType.Type.GetChildInfo("material");

            terrainBaseTextureStrataType.Type = getNodeType("gap", "terrainBaseTextureStrataType");
            terrainBaseTextureStrataType.texture0Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("texture0");
            terrainBaseTextureStrataType.texture1Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("texture1");
            terrainBaseTextureStrataType.texture2Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("texture2");
            terrainBaseTextureStrataType.mapping0Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("mapping0");
            terrainBaseTextureStrataType.mapping1Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("mapping1");
            terrainBaseTextureStrataType.mapping2Attribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("mapping2");
            terrainBaseTextureStrataType.endheightAttribute = terrainBaseTextureStrataType.Type.GetAttributeInfo("endheight");

            terrainStrataMaterialType.Type = getNodeType("gap", "terrainStrataMaterialType");
            terrainStrataMaterialType.MaterialIdAttribute = terrainStrataMaterialType.Type.GetAttributeInfo("MaterialId");
            terrainStrataMaterialType.strataChild = terrainStrataMaterialType.Type.GetChildInfo("strata");

            terrainMaterialType.Type = getNodeType("gap", "terrainMaterialType");
            terrainMaterialType.MaterialIdAttribute = terrainMaterialType.Type.GetAttributeInfo("MaterialId");
            terrainMaterialType.FlatTextureAttribute = terrainMaterialType.Type.GetAttributeInfo("FlatTexture");
            terrainMaterialType.SlopeTexture0Attribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture0");
            terrainMaterialType.SlopeTexture1Attribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture1");
            terrainMaterialType.SlopeTexture2Attribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture2");
            terrainMaterialType.BlendingTextureAttribute = terrainMaterialType.Type.GetAttributeInfo("BlendingTexture");
            terrainMaterialType.FlatTextureMappingAttribute = terrainMaterialType.Type.GetAttributeInfo("FlatTextureMapping");
            terrainMaterialType.SlopeTexture0MappingAttribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture0Mapping");
            terrainMaterialType.SlopeTexture1MappingAttribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture1Mapping");
            terrainMaterialType.SlopeTexture2MappingAttribute = terrainMaterialType.Type.GetAttributeInfo("SlopeTexture2Mapping");
            terrainMaterialType.BlendingTextureMappingAttribute = terrainMaterialType.Type.GetAttributeInfo("BlendingTextureMapping");

            terrainProcTextureType.Type = getNodeType("gap", "terrainProcTextureType");
            terrainProcTextureType.MaterialIdAttribute = terrainProcTextureType.Type.GetAttributeInfo("MaterialId");
            terrainProcTextureType.NameAttribute = terrainProcTextureType.Type.GetAttributeInfo("Name");
            terrainProcTextureType.Texture0Attribute = terrainProcTextureType.Type.GetAttributeInfo("Texture0");
            terrainProcTextureType.Texture1Attribute = terrainProcTextureType.Type.GetAttributeInfo("Texture1");
            terrainProcTextureType.HGridAttribute = terrainProcTextureType.Type.GetAttributeInfo("HGrid");
            terrainProcTextureType.GainAttribute = terrainProcTextureType.Type.GetAttributeInfo("Gain");

            vegetationSpawnObjectType.Type = getNodeType("gap", "vegetationSpawnObjectType");
            vegetationSpawnObjectType.MaxDrawDistanceAttribute = vegetationSpawnObjectType.Type.GetAttributeInfo("MaxDrawDistance");
            vegetationSpawnObjectType.FrequencyWeightAttribute = vegetationSpawnObjectType.Type.GetAttributeInfo("FrequencyWeight");
            vegetationSpawnObjectType.ModelAttribute = vegetationSpawnObjectType.Type.GetAttributeInfo("Model");
            vegetationSpawnObjectType.MaterialAttribute = vegetationSpawnObjectType.Type.GetAttributeInfo("Material");

            vegetationSpawnMaterialType.Type = getNodeType("gap", "vegetationSpawnMaterialType");
            vegetationSpawnMaterialType.NoSpawnWeightAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("NoSpawnWeight");
            vegetationSpawnMaterialType.SuppressionThresholdAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("SuppressionThreshold");
            vegetationSpawnMaterialType.SuppressionNoiseAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("SuppressionNoise");
            vegetationSpawnMaterialType.SuppressionGainAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("SuppressionGain");
            vegetationSpawnMaterialType.SuppressionLacunarityAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("SuppressionLacunarity");
            vegetationSpawnMaterialType.MaterialIdAttribute = vegetationSpawnMaterialType.Type.GetAttributeInfo("MaterialId");
            vegetationSpawnMaterialType.objectChild = vegetationSpawnMaterialType.Type.GetChildInfo("object");

            vegetationSpawnConfigType.Type = getNodeType("gap", "vegetationSpawnConfigType");
            vegetationSpawnConfigType.BaseGridSpacingAttribute = vegetationSpawnConfigType.Type.GetAttributeInfo("BaseGridSpacing");
            vegetationSpawnConfigType.JitterAmountAttribute = vegetationSpawnConfigType.Type.GetAttributeInfo("JitterAmount");
            vegetationSpawnConfigType.materialChild = vegetationSpawnConfigType.Type.GetChildInfo("material");

            terrainCoverageLayer.Type = getNodeType("gap", "terrainCoverageLayer");
            terrainCoverageLayer.IdAttribute = terrainCoverageLayer.Type.GetAttributeInfo("Id");
            terrainCoverageLayer.ResolutionAttribute = terrainCoverageLayer.Type.GetAttributeInfo("Resolution");
            terrainCoverageLayer.OverlapAttribute = terrainCoverageLayer.Type.GetAttributeInfo("Overlap");
            terrainCoverageLayer.SourceFileAttribute = terrainCoverageLayer.Type.GetAttributeInfo("SourceFile");
            terrainCoverageLayer.EnableAttribute = terrainCoverageLayer.Type.GetAttributeInfo("Enable");
            terrainCoverageLayer.FormatAttribute = terrainCoverageLayer.Type.GetAttributeInfo("Format");
            terrainCoverageLayer.ShaderNormalizationModeAttribute = terrainCoverageLayer.Type.GetAttributeInfo("ShaderNormalizationMode");

            terrainType.Type = getNodeType("gap", "terrainType");
            terrainType.UberSurfaceDirAttribute = terrainType.Type.GetAttributeInfo("UberSurfaceDir");
            terrainType.CellsDirAttribute = terrainType.Type.GetAttributeInfo("CellsDir");
            terrainType.ConfigFileTargetAttribute = terrainType.Type.GetAttributeInfo("ConfigFileTarget");
            terrainType.NodeDimensionsAttribute = terrainType.Type.GetAttributeInfo("NodeDimensions");
            terrainType.OverlapAttribute = terrainType.Type.GetAttributeInfo("Overlap");
            terrainType.SpacingAttribute = terrainType.Type.GetAttributeInfo("Spacing");
            terrainType.CellTreeDepthAttribute = terrainType.Type.GetAttributeInfo("CellTreeDepth");
            terrainType.CellCountAttribute = terrainType.Type.GetAttributeInfo("CellCount");
            terrainType.OffsetAttribute = terrainType.Type.GetAttributeInfo("Offset");
            terrainType.HasEncodedGradientFlagsAttribute = terrainType.Type.GetAttributeInfo("HasEncodedGradientFlags");
            terrainType.GradFlagSlopeThreshold0Attribute = terrainType.Type.GetAttributeInfo("GradFlagSlopeThreshold0");
            terrainType.GradFlagSlopeThreshold1Attribute = terrainType.Type.GetAttributeInfo("GradFlagSlopeThreshold1");
            terrainType.GradFlagSlopeThreshold2Attribute = terrainType.Type.GetAttributeInfo("GradFlagSlopeThreshold2");
            terrainType.SunPathAngleAttribute = terrainType.Type.GetAttributeInfo("SunPathAngle");
            terrainType.baseTextureChild = terrainType.Type.GetChildInfo("baseTexture");
            terrainType.VegetationSpawnChild = terrainType.Type.GetChildInfo("VegetationSpawn");
            terrainType.coverageChild = terrainType.Type.GetChildInfo("coverage");

            resourceReferenceType.Type = getNodeType("gap", "resourceReferenceType");
            resourceReferenceType.uriAttribute = resourceReferenceType.Type.GetAttributeInfo("uri");

            gameObjectComponentType.Type = getNodeType("gap", "gameObjectComponentType");
            gameObjectComponentType.nameAttribute = gameObjectComponentType.Type.GetAttributeInfo("name");
            gameObjectComponentType.activeAttribute = gameObjectComponentType.Type.GetAttributeInfo("active");

            transformComponentType.Type = getNodeType("gap", "transformComponentType");
            transformComponentType.nameAttribute = transformComponentType.Type.GetAttributeInfo("name");
            transformComponentType.activeAttribute = transformComponentType.Type.GetAttributeInfo("active");
            transformComponentType.translationAttribute = transformComponentType.Type.GetAttributeInfo("translation");
            transformComponentType.rotationAttribute = transformComponentType.Type.GetAttributeInfo("rotation");
            transformComponentType.scaleAttribute = transformComponentType.Type.GetAttributeInfo("scale");

            gameObjectWithComponentType.Type = getNodeType("gap", "gameObjectWithComponentType");
            gameObjectWithComponentType.transformAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("transform");
            gameObjectWithComponentType.translateAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("translate");
            gameObjectWithComponentType.rotateAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("rotate");
            gameObjectWithComponentType.scaleAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("scale");
            gameObjectWithComponentType.pivotAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("pivot");
            gameObjectWithComponentType.transformationTypeAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("transformationType");
            gameObjectWithComponentType.nameAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("name");
            gameObjectWithComponentType.visibleAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("visible");
            gameObjectWithComponentType.lockedAttribute = gameObjectWithComponentType.Type.GetAttributeInfo("locked");
            gameObjectWithComponentType.componentChild = gameObjectWithComponentType.Type.GetChildInfo("component");

            gameObjectGroupType.Type = getNodeType("gap", "gameObjectGroupType");
            gameObjectGroupType.transformAttribute = gameObjectGroupType.Type.GetAttributeInfo("transform");
            gameObjectGroupType.translateAttribute = gameObjectGroupType.Type.GetAttributeInfo("translate");
            gameObjectGroupType.rotateAttribute = gameObjectGroupType.Type.GetAttributeInfo("rotate");
            gameObjectGroupType.scaleAttribute = gameObjectGroupType.Type.GetAttributeInfo("scale");
            gameObjectGroupType.pivotAttribute = gameObjectGroupType.Type.GetAttributeInfo("pivot");
            gameObjectGroupType.transformationTypeAttribute = gameObjectGroupType.Type.GetAttributeInfo("transformationType");
            gameObjectGroupType.nameAttribute = gameObjectGroupType.Type.GetAttributeInfo("name");
            gameObjectGroupType.visibleAttribute = gameObjectGroupType.Type.GetAttributeInfo("visible");
            gameObjectGroupType.lockedAttribute = gameObjectGroupType.Type.GetAttributeInfo("locked");
            gameObjectGroupType.gameObjectChild = gameObjectGroupType.Type.GetChildInfo("gameObject");

            markerPointType.Type = getNodeType("gap", "markerPointType");
            markerPointType.translateAttribute = markerPointType.Type.GetAttributeInfo("translate");

            triMeshMarkerType.Type = getNodeType("gap", "triMeshMarkerType");
            triMeshMarkerType.transformAttribute = triMeshMarkerType.Type.GetAttributeInfo("transform");
            triMeshMarkerType.translateAttribute = triMeshMarkerType.Type.GetAttributeInfo("translate");
            triMeshMarkerType.rotateAttribute = triMeshMarkerType.Type.GetAttributeInfo("rotate");
            triMeshMarkerType.scaleAttribute = triMeshMarkerType.Type.GetAttributeInfo("scale");
            triMeshMarkerType.pivotAttribute = triMeshMarkerType.Type.GetAttributeInfo("pivot");
            triMeshMarkerType.transformationTypeAttribute = triMeshMarkerType.Type.GetAttributeInfo("transformationType");
            triMeshMarkerType.nameAttribute = triMeshMarkerType.Type.GetAttributeInfo("name");
            triMeshMarkerType.visibleAttribute = triMeshMarkerType.Type.GetAttributeInfo("visible");
            triMeshMarkerType.lockedAttribute = triMeshMarkerType.Type.GetAttributeInfo("locked");
            triMeshMarkerType.indexlistAttribute = triMeshMarkerType.Type.GetAttributeInfo("indexlist");
            triMeshMarkerType.ShowMarkerAttribute = triMeshMarkerType.Type.GetAttributeInfo("ShowMarker");
            triMeshMarkerType.pointsChild = triMeshMarkerType.Type.GetChildInfo("points");

            xleGameType.Type = getNodeType("gap", "xleGameType");
            xleGameType.nameAttribute = xleGameType.Type.GetAttributeInfo("name");
            xleGameType.fogEnabledAttribute = xleGameType.Type.GetAttributeInfo("fogEnabled");
            xleGameType.fogColorAttribute = xleGameType.Type.GetAttributeInfo("fogColor");
            xleGameType.fogRangeAttribute = xleGameType.Type.GetAttributeInfo("fogRange");
            xleGameType.fogDensityAttribute = xleGameType.Type.GetAttributeInfo("fogDensity");
            xleGameType.ExportDirectoryAttribute = xleGameType.Type.GetAttributeInfo("ExportDirectory");
            xleGameType.gameObjectFolderChild = xleGameType.Type.GetChildInfo("gameObjectFolder");
            xleGameType.layersChild = xleGameType.Type.GetChildInfo("layers");
            xleGameType.bookmarksChild = xleGameType.Type.GetChildInfo("bookmarks");
            xleGameType.gameReferenceChild = xleGameType.Type.GetChildInfo("gameReference");
            xleGameType.gridChild = xleGameType.Type.GetChildInfo("grid");
            xleGameType.placementsChild = xleGameType.Type.GetChildInfo("placements");
            xleGameType.terrainChild = xleGameType.Type.GetChildInfo("terrain");
            xleGameType.environmentChild = xleGameType.Type.GetChildInfo("environment");

            placementsDocumentRootElement = getRootElement(NS, "placementsDocument");
            gameRootElement = getRootElement(NS, "game");
        }

        public static class placementsDocumentType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static ChildInfo placementChild;
        }

        public static class abstractPlacementObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo IDAttribute;
        }

        public static class transformObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
        }

        public static class gameType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo fogEnabledAttribute;
            public static AttributeInfo fogColorAttribute;
            public static AttributeInfo fogRangeAttribute;
            public static AttributeInfo fogDensityAttribute;
            public static ChildInfo gameObjectFolderChild;
            public static ChildInfo layersChild;
            public static ChildInfo bookmarksChild;
            public static ChildInfo gameReferenceChild;
            public static ChildInfo gridChild;
        }

        public static class gameObjectFolderType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static ChildInfo gameObjectChild;
            public static ChildInfo folderChild;
        }

        public static class gameObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
        }

        public static class layersType
        {
            public static DomNodeType Type;
            public static ChildInfo layerChild;
        }

        public static class layerType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static ChildInfo gameObjectReferenceChild;
            public static ChildInfo layerChild;
        }

        public static class gameObjectReferenceType
        {
            public static DomNodeType Type;
            public static AttributeInfo refAttribute;
        }

        public static class bookmarksType
        {
            public static DomNodeType Type;
            public static ChildInfo bookmarkChild;
        }

        public static class bookmarkType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static ChildInfo cameraChild;
            public static ChildInfo bookmarkChild;
        }

        public static class cameraType
        {
            public static DomNodeType Type;
            public static AttributeInfo eyeAttribute;
            public static AttributeInfo lookAtPointAttribute;
            public static AttributeInfo upVectorAttribute;
            public static AttributeInfo viewTypeAttribute;
            public static AttributeInfo yFovAttribute;
            public static AttributeInfo nearZAttribute;
            public static AttributeInfo farZAttribute;
            public static AttributeInfo focusRadiusAttribute;
        }

        public static class gameReferenceType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo refAttribute;
            public static AttributeInfo tagsAttribute;
        }

        public static class gridType
        {
            public static DomNodeType Type;
            public static AttributeInfo sizeAttribute;
            public static AttributeInfo subdivisionsAttribute;
            public static AttributeInfo heightAttribute;
            public static AttributeInfo snapAttribute;
            public static AttributeInfo visibleAttribute;
        }

        public static class ambientSettingsType
        {
            public static DomNodeType Type;
            public static AttributeInfo AmbientLightAttribute;
            public static AttributeInfo AmbientBrightnessAttribute;
            public static AttributeInfo SkyTextureAttribute;
            public static AttributeInfo SkyReflectionScaleAttribute;
            public static AttributeInfo SkyReflectionBlurrinessAttribute;
            public static AttributeInfo SkyBrightnessAttribute;
            public static AttributeInfo RangeFogInscatterAttribute;
            public static AttributeInfo RangeFogInscatterScaleAttribute;
            public static AttributeInfo RangeFogThicknessAttribute;
            public static AttributeInfo RangeFogThicknessScaleAttribute;
            public static AttributeInfo AtmosBlurStdDevAttribute;
            public static AttributeInfo AtmosBlurStartAttribute;
            public static AttributeInfo AtmosBlurEndAttribute;
            public static AttributeInfo FlagsAttribute;
        }

        public static class toneMapSettingsType
        {
            public static DomNodeType Type;
            public static AttributeInfo BloomScaleAttribute;
            public static AttributeInfo BloomThresholdAttribute;
            public static AttributeInfo BloomRampingFactorAttribute;
            public static AttributeInfo BloomDesaturationFactorAttribute;
            public static AttributeInfo BloomBlurStdDevAttribute;
            public static AttributeInfo BloomBrightnessAttribute;
            public static AttributeInfo SceneKeyAttribute;
            public static AttributeInfo LuminanceMinAttribute;
            public static AttributeInfo LuminanceMaxAttribute;
            public static AttributeInfo WhitePointAttribute;
            public static AttributeInfo FlagsAttribute;
        }

        public static class envObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
        }

        public static class envMiscType
        {
            public static DomNodeType Type;
        }

        public static class envSettingsType
        {
            public static DomNodeType Type;
            public static AttributeInfo NameAttribute;
            public static ChildInfo objectsChild;
            public static ChildInfo settingsChild;
            public static ChildInfo ambientChild;
            public static ChildInfo tonemapChild;
        }

        public static class envSettingsFolderType
        {
            public static DomNodeType Type;
            public static AttributeInfo ExportTargetAttribute;
            public static AttributeInfo ExportEnabledAttribute;
            public static ChildInfo settingsChild;
        }

        public static class directionalLightType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo DiffuseAttribute;
            public static AttributeInfo DiffuseBrightnessAttribute;
            public static AttributeInfo DiffuseModelAttribute;
            public static AttributeInfo DiffuseWideningMinAttribute;
            public static AttributeInfo DiffuseWideningMaxAttribute;
            public static AttributeInfo SpecularAttribute;
            public static AttributeInfo SpecularBrightnessAttribute;
            public static AttributeInfo SpecularNonMetalBrightnessAttribute;
            public static AttributeInfo FlagsAttribute;
            public static AttributeInfo ShadowResolveModelAttribute;
        }

        public static class shadowFrustumSettings
        {
            public static DomNodeType Type;
            public static AttributeInfo NameAttribute;
            public static AttributeInfo FlagsAttribute;
            public static AttributeInfo FrustumCountAttribute;
            public static AttributeInfo MaxDistanceFromCameraAttribute;
            public static AttributeInfo FrustumSizeFactorAttribute;
            public static AttributeInfo FocusDistanceAttribute;
            public static AttributeInfo TextureSizeAttribute;
            public static AttributeInfo ShadowSlopeScaledBiasAttribute;
            public static AttributeInfo ShadowDepthBiasClampAttribute;
            public static AttributeInfo ShadowRasterDepthBiasAttribute;
            public static AttributeInfo WorldSpaceResolveBiasAttribute;
            public static AttributeInfo BlurAngleDegreesAttribute;
            public static AttributeInfo MinBlurSearchAttribute;
            public static AttributeInfo MaxBlurSearchAttribute;
            public static AttributeInfo LightAttribute;
        }

        public static class oceanSettings
        {
            public static DomNodeType Type;
            public static AttributeInfo EnableAttribute;
            public static AttributeInfo WindAngleAttribute;
            public static AttributeInfo WindVelocityAttribute;
            public static AttributeInfo PhysicalDimensionsAttribute;
            public static AttributeInfo GridDimensionsAttribute;
            public static AttributeInfo StrengthConstantXYAttribute;
            public static AttributeInfo StrengthConstantZAttribute;
            public static AttributeInfo DetailNormalsStrengthAttribute;
            public static AttributeInfo SpectrumFadeAttribute;
            public static AttributeInfo ScaleAgainstWindAttribute;
            public static AttributeInfo SuppressionFactorAttribute;
            public static AttributeInfo GridShiftSpeedAttribute;
            public static AttributeInfo BaseHeightAttribute;
            public static AttributeInfo FoamThresholdAttribute;
            public static AttributeInfo FoamIncreaseSpeedAttribute;
            public static AttributeInfo FoamIncreaseClampAttribute;
            public static AttributeInfo FoamDecreaseAttribute;
        }

        public static class oceanLightingSettings
        {
            public static DomNodeType Type;
            public static AttributeInfo FoamBrightnessAttribute;
            public static AttributeInfo OpticalThicknessScalarAttribute;
            public static AttributeInfo OpticalThicknessColorAttribute;
            public static AttributeInfo SkyReflectionBrightnessAttribute;
            public static AttributeInfo UpwellingScaleAttribute;
            public static AttributeInfo RefractiveIndexAttribute;
            public static AttributeInfo ReflectionBumpScaleAttribute;
            public static AttributeInfo DetailNormalFrequencyAttribute;
            public static AttributeInfo SpecularityFrequencyAttribute;
            public static AttributeInfo MatSpecularMinAttribute;
            public static AttributeInfo MatSpecularMaxAttribute;
            public static AttributeInfo MatRoughnessAttribute;
        }

        public static class envUtilityType
        {
            public static DomNodeType Type;
            public static AttributeInfo SunAngleAttribute;
            public static AttributeInfo SunNameAttribute;
        }

        public static class fogVolumeType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo DensityAttribute;
            public static AttributeInfo NoiseDensityScaleAttribute;
            public static AttributeInfo NoiseSpeedAttribute;
            public static AttributeInfo ForwardColorAttribute;
            public static AttributeInfo ForwardColorScaleAttribute;
            public static AttributeInfo BackColorAttribute;
            public static AttributeInfo BackColorScaleAttribute;
            public static AttributeInfo ESM_CAttribute;
            public static AttributeInfo ShadowBiasAttribute;
            public static AttributeInfo JitteringAmountAttribute;
            public static AttributeInfo HeightStartAttribute;
            public static AttributeInfo HeightEndAttribute;
        }

        public static class fogVolumeRendererType
        {
            public static DomNodeType Type;
            public static AttributeInfo BlurredShadowSizeAttribute;
            public static AttributeInfo ShadowDownsampleAttribute;
            public static AttributeInfo SkipShadowFrustumsAttribute;
            public static AttributeInfo MaxShadowFrustumsAttribute;
            public static AttributeInfo GridDimensionsAttribute;
            public static AttributeInfo WorldSpaceGridDepthAttribute;
        }

        public static class shallowSurfaceType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo MarkerAttribute;
            public static AttributeInfo GridPhysicalSizeAttribute;
            public static AttributeInfo GridDimsAttribute;
            public static AttributeInfo SimGridCountAttribute;
            public static AttributeInfo BaseHeightAttribute;
            public static AttributeInfo SimMethodAttribute;
            public static AttributeInfo RainQuantityAttribute;
            public static AttributeInfo EvaporationConstantAttribute;
            public static AttributeInfo PressureConstantAttribute;
            public static AttributeInfo OpticalThicknessColorAttribute;
            public static AttributeInfo OpticalThicknessScalarAttribute;
            public static AttributeInfo FoamColorAttribute;
            public static AttributeInfo SpecularAttribute;
            public static AttributeInfo RoughnessAttribute;
            public static AttributeInfo RefractiveIndexAttribute;
            public static AttributeInfo UpwellingScaleAttribute;
            public static AttributeInfo SkyReflectionScaleAttribute;
        }

        public static class placementsCellReferenceType
        {
            public static DomNodeType Type;
            public static AttributeInfo refAttribute;
            public static AttributeInfo ExportTargetAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo captureMinsAttribute;
            public static AttributeInfo captureMaxsAttribute;
            public static AttributeInfo offsetAttribute;
            public static AttributeInfo ExportEnabledAttribute;
            public static AttributeInfo cachedCellMinsAttribute;
            public static AttributeInfo cachedCellMaxsAttribute;
        }

        public static class placementsFolderType
        {
            public static DomNodeType Type;
            public static AttributeInfo baseEditorPathAttribute;
            public static AttributeInfo baseExportPathAttribute;
            public static AttributeInfo CellCountAttribute;
            public static AttributeInfo CellsOriginAttribute;
            public static AttributeInfo CellSizeAttribute;
            public static AttributeInfo ExportTargetAttribute;
            public static AttributeInfo ExportEnabledAttribute;
            public static ChildInfo cellChild;
        }

        public static class placementObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo IDAttribute;
            public static AttributeInfo modelAttribute;
            public static AttributeInfo materialAttribute;
        }

        public static class abstractTerrainMaterialDescType
        {
            public static DomNodeType Type;
            public static AttributeInfo MaterialIdAttribute;
        }

        public static class terrainBaseTextureType
        {
            public static DomNodeType Type;
            public static AttributeInfo diffusedimsAttribute;
            public static AttributeInfo normaldimsAttribute;
            public static AttributeInfo paramdimsAttribute;
            public static ChildInfo materialChild;
        }

        public static class terrainBaseTextureStrataType
        {
            public static DomNodeType Type;
            public static AttributeInfo texture0Attribute;
            public static AttributeInfo texture1Attribute;
            public static AttributeInfo texture2Attribute;
            public static AttributeInfo mapping0Attribute;
            public static AttributeInfo mapping1Attribute;
            public static AttributeInfo mapping2Attribute;
            public static AttributeInfo endheightAttribute;
        }

        public static class terrainStrataMaterialType
        {
            public static DomNodeType Type;
            public static AttributeInfo MaterialIdAttribute;
            public static ChildInfo strataChild;
        }

        public static class terrainMaterialType
        {
            public static DomNodeType Type;
            public static AttributeInfo MaterialIdAttribute;
            public static AttributeInfo FlatTextureAttribute;
            public static AttributeInfo SlopeTexture0Attribute;
            public static AttributeInfo SlopeTexture1Attribute;
            public static AttributeInfo SlopeTexture2Attribute;
            public static AttributeInfo BlendingTextureAttribute;
            public static AttributeInfo FlatTextureMappingAttribute;
            public static AttributeInfo SlopeTexture0MappingAttribute;
            public static AttributeInfo SlopeTexture1MappingAttribute;
            public static AttributeInfo SlopeTexture2MappingAttribute;
            public static AttributeInfo BlendingTextureMappingAttribute;
        }

        public static class terrainProcTextureType
        {
            public static DomNodeType Type;
            public static AttributeInfo MaterialIdAttribute;
            public static AttributeInfo NameAttribute;
            public static AttributeInfo Texture0Attribute;
            public static AttributeInfo Texture1Attribute;
            public static AttributeInfo HGridAttribute;
            public static AttributeInfo GainAttribute;
        }

        public static class vegetationSpawnObjectType
        {
            public static DomNodeType Type;
            public static AttributeInfo MaxDrawDistanceAttribute;
            public static AttributeInfo FrequencyWeightAttribute;
            public static AttributeInfo ModelAttribute;
            public static AttributeInfo MaterialAttribute;
        }

        public static class vegetationSpawnMaterialType
        {
            public static DomNodeType Type;
            public static AttributeInfo NoSpawnWeightAttribute;
            public static AttributeInfo SuppressionThresholdAttribute;
            public static AttributeInfo SuppressionNoiseAttribute;
            public static AttributeInfo SuppressionGainAttribute;
            public static AttributeInfo SuppressionLacunarityAttribute;
            public static AttributeInfo MaterialIdAttribute;
            public static ChildInfo objectChild;
        }

        public static class vegetationSpawnConfigType
        {
            public static DomNodeType Type;
            public static AttributeInfo BaseGridSpacingAttribute;
            public static AttributeInfo JitterAmountAttribute;
            public static ChildInfo materialChild;
        }

        public static class terrainCoverageLayer
        {
            public static DomNodeType Type;
            public static AttributeInfo IdAttribute;
            public static AttributeInfo ResolutionAttribute;
            public static AttributeInfo OverlapAttribute;
            public static AttributeInfo SourceFileAttribute;
            public static AttributeInfo EnableAttribute;
            public static AttributeInfo FormatAttribute;
            public static AttributeInfo ShaderNormalizationModeAttribute;
        }

        public static class terrainType
        {
            public static DomNodeType Type;
            public static AttributeInfo UberSurfaceDirAttribute;
            public static AttributeInfo CellsDirAttribute;
            public static AttributeInfo ConfigFileTargetAttribute;
            public static AttributeInfo NodeDimensionsAttribute;
            public static AttributeInfo OverlapAttribute;
            public static AttributeInfo SpacingAttribute;
            public static AttributeInfo CellTreeDepthAttribute;
            public static AttributeInfo CellCountAttribute;
            public static AttributeInfo OffsetAttribute;
            public static AttributeInfo HasEncodedGradientFlagsAttribute;
            public static AttributeInfo GradFlagSlopeThreshold0Attribute;
            public static AttributeInfo GradFlagSlopeThreshold1Attribute;
            public static AttributeInfo GradFlagSlopeThreshold2Attribute;
            public static AttributeInfo SunPathAngleAttribute;
            public static ChildInfo baseTextureChild;
            public static ChildInfo VegetationSpawnChild;
            public static ChildInfo coverageChild;
        }

        public static class resourceReferenceType
        {
            public static DomNodeType Type;
            public static AttributeInfo uriAttribute;
        }

        public static class gameObjectComponentType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo activeAttribute;
        }

        public static class transformComponentType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo activeAttribute;
            public static AttributeInfo translationAttribute;
            public static AttributeInfo rotationAttribute;
            public static AttributeInfo scaleAttribute;
        }

        public static class gameObjectWithComponentType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static ChildInfo componentChild;
        }

        public static class gameObjectGroupType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static ChildInfo gameObjectChild;
        }

        public static class markerPointType
        {
            public static DomNodeType Type;
            public static AttributeInfo translateAttribute;
        }

        public static class triMeshMarkerType
        {
            public static DomNodeType Type;
            public static AttributeInfo transformAttribute;
            public static AttributeInfo translateAttribute;
            public static AttributeInfo rotateAttribute;
            public static AttributeInfo scaleAttribute;
            public static AttributeInfo pivotAttribute;
            public static AttributeInfo transformationTypeAttribute;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo visibleAttribute;
            public static AttributeInfo lockedAttribute;
            public static AttributeInfo indexlistAttribute;
            public static AttributeInfo ShowMarkerAttribute;
            public static ChildInfo pointsChild;
        }

        public static class xleGameType
        {
            public static DomNodeType Type;
            public static AttributeInfo nameAttribute;
            public static AttributeInfo fogEnabledAttribute;
            public static AttributeInfo fogColorAttribute;
            public static AttributeInfo fogRangeAttribute;
            public static AttributeInfo fogDensityAttribute;
            public static AttributeInfo ExportDirectoryAttribute;
            public static ChildInfo gameObjectFolderChild;
            public static ChildInfo layersChild;
            public static ChildInfo bookmarksChild;
            public static ChildInfo gameReferenceChild;
            public static ChildInfo gridChild;
            public static ChildInfo placementsChild;
            public static ChildInfo terrainChild;
            public static ChildInfo environmentChild;
        }

        public static ChildInfo placementsDocumentRootElement;

        public static ChildInfo gameRootElement;
    }
}
