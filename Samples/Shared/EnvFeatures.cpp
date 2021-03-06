// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "EnvFeatures.h"
#include "../../SceneEngine/LightingParserContext.h"
#include "../../SceneEngine/SceneParser.h"
#include "../../SceneEngine/VolumetricFog.h"
#include "../../SceneEngine/ShallowSurface.h"
#include "../../SceneEngine/VegetationSpawn.h"
#include "../../Tools/EntityInterface/EnvironmentSettings.h"
#include "../../Tools/EntityInterface/RetainedEntities.h"
#include "../../Assets/ConfigFileContainer.h"

namespace Sample
{
    static const ::Assets::ResChar VegetationSpawnCfg[] = "vegetationspawn.cfg";

    void ScenePlugin_EnvironmentFeatures::LoadingPhase()
    {
        _updateMan->FlushUpdates();

        if (_vegetationSpawnManager && (!_vegetationSpawnCfgVal || _vegetationSpawnCfgVal->GetValidationIndex() != 0)) {
            ::Assets::ConfigFileContainer<SceneEngine::VegetationSpawnConfig> container((_cfgDir + VegetationSpawnCfg).c_str());
            _vegetationSpawnManager->Load(container._asset);
            _vegetationSpawnCfgVal = container.GetDependencyValidation();
        }
    }

    void ScenePlugin_EnvironmentFeatures::PrepareFrame(
        RenderCore::IThreadContext& threadContext,
        SceneEngine::LightingParserContext& parserContext)
    {
        if (_vegetationSpawnManager)
            parserContext._plugins.push_back(_vegetationSpawnManager->GetParserPlugin());
        parserContext._plugins.push_back(_volumetricFogMan->GetParserPlugin());
    }

    void ScenePlugin_EnvironmentFeatures::ExecuteScene(   
        RenderCore::Metal::DeviceContext* context, 
        SceneEngine::LightingParserContext& parserContext, 
        const SceneEngine::SceneParseSettings& parseSettings,
        unsigned techniqueIndex) const
    {
        using BatchFilter = SceneEngine::SceneParseSettings::BatchFilter;
        using Toggles = SceneEngine::SceneParseSettings::Toggles;

        if (    parseSettings._batchFilter == BatchFilter::General
            ||  parseSettings._batchFilter == BatchFilter::Depth) {

            if (parseSettings._toggles & Toggles::NonTerrain) {
                _vegetationSpawnManager->Render(context, parserContext, techniqueIndex);
            }
        } else if (parseSettings._batchFilter == BatchFilter::Transparent) {
            if (parseSettings._toggles & Toggles::NonTerrain) {
                if (_shallowSurfaces)
                    _shallowSurfaces->RenderDebugging(*context, parserContext, techniqueIndex, _surfaceHeights.get());
            }
        }
    }

    ScenePlugin_EnvironmentFeatures::ScenePlugin_EnvironmentFeatures(
        const ::Assets::rstring& cfgDir,
        std::shared_ptr<EntityInterface::RetainedEntities> retainedEntities,
        std::shared_ptr<RenderCore::Assets::ModelCache> modelCache,
        std::shared_ptr<SceneEngine::ISurfaceHeightsProvider> surfaceHeights)
    : _cfgDir(cfgDir)
    {
        _volumetricFogMan = std::make_shared<SceneEngine::VolumetricFogManager>();
        _shallowSurfaces = std::make_shared<SceneEngine::ShallowSurfaceManager>();
        _vegetationSpawnManager = std::make_shared<SceneEngine::VegetationSpawnManager>(std::move(modelCache));
        _surfaceHeights = std::move(surfaceHeights);

            //  Create the object that will manage updates via the entities
            //  interface system.
            //  Then register update events for our scene engine managers.
            //  When relevant game objects are loaded, the entities interface
            //  will push update information through to these managers.
        _updateMan = std::make_shared<EntityInterface::EnvEntitiesManager>(std::move(retainedEntities));
        _updateMan->RegisterVolumetricFogFlexObjects(_volumetricFogMan);
        _updateMan->RegisterShallowSurfaceFlexObjects(_shallowSurfaces);

            // Volume Fog renderer configuration must come from the environment
            // settings... But no access to that here...?
    }

    ScenePlugin_EnvironmentFeatures::~ScenePlugin_EnvironmentFeatures()
    {}

///////////////////////////////////////////////////////////////////////////////////////////////////

    IScenePlugin::~IScenePlugin() {}

}

