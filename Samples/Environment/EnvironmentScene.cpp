// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "EnvironmentScene.h"
#include "../Shared/CharactersScene.h"
#include "../Shared/SampleGlobals.h"
#include "../Shared/IScenePlugin.h"
#include "../Shared/EnvFeatures.h"

#include "../../PlatformRig/PlatformRigUtil.h"

#include "../../SceneEngine/LightingParserContext.h"
#include "../../SceneEngine/Terrain.h"
#include "../../SceneEngine/PlacementsManager.h"
#include "../../SceneEngine/TerrainFormat.h"
#include "../../SceneEngine/TerrainConfig.h"
#include "../../SceneEngine/TerrainMaterial.h"

#include "../../RenderCore/Assets/ModelCache.h"
#include "../../Tools/EntityInterface/RetainedEntities.h"

#include "../../Assets/ConfigFileContainer.h"
#include "../../ConsoleRig/Console.h"
#include "../../Utility/Profiling/CPUProfiler.h"
#include "../../Utility/Streams/PathUtils.h"

namespace Sample
{
    static const char* WorldDirectory = "game/demworld";

    std::shared_ptr<SceneEngine::ITerrainFormat>     MainTerrainFormat;
    SceneEngine::TerrainCoordinateSystem             MainTerrainCoords;
    SceneEngine::TerrainConfig                       MainTerrainConfig;

///////////////////////////////////////////////////////////////////////////////////////////////////

    class EnvironmentSceneParser::Pimpl
    {
    public:
        std::unique_ptr<CharactersScene>                    _characters;
        std::shared_ptr<SceneEngine::TerrainManager>        _terrainManager;
        std::shared_ptr<SceneEngine::PlacementsManager>     _placementsManager;
        std::shared_ptr<RenderCore::Techniques::CameraDesc> _cameraDesc;
        std::shared_ptr<RenderCore::Assets::ModelCache>     _modelCache;
        PlatformRig::EnvironmentSettings                    _envSettings;

        std::shared_ptr<::Assets::DependencyValidation>     _terrainCfgVal;
        std::shared_ptr<::Assets::DependencyValidation>     _terrainTexturesCfgVal;
        std::shared_ptr<::Assets::DependencyValidation>     _placementsCfgVal;
        std::shared_ptr<::Assets::DependencyValidation>     _environmentCfgVal;
        std::shared_ptr<::Assets::DependencyValidation>     _gameObjectsCfgVal;

        std::shared_ptr<EntityInterface::RetainedEntities>  _retainedEntities;

        std::vector<std::shared_ptr<IScenePlugin>>          _scenePlugins;

        ::Assets::rstring _cfgDir;
        float _time;

        void LoadGameObjects(const ::Assets::ResChar filename[]);

        ::Assets::rstring MakeCfgName(const ::Assets::ResChar cfg[])
        {
            return _cfgDir + cfg;
        }

    };

    void EnvironmentSceneParser::Pimpl::LoadGameObjects(const ::Assets::ResChar filename[])
    {
        size_t fileSize = 0;
        auto sourceFile = LoadFileAsMemoryBlock(filename, &fileSize);
        using Formatter = InputStreamFormatter<utf8>;
        Formatter formatter(
            MemoryMappedInputStream(sourceFile.get(), PtrAdd(sourceFile.get(), fileSize)));

            // Parse the list of game objects, and create true native objects
            // as required.
            //
            // Note that this path is designed to be generic and flexible...
            // A final solution would do a lot of this work in the "export" step
            // of the editor -- and write out objects that are more closely
            // associated with the native types.
            //
            // But of this sample, we'll just use a simple generic approach.
            // We'll use the EntityInterface library -- this is the library
            // that is used to drive the native objects in the editor.
            // The EntityInterface provides a layer
            // over many native engine types, so they appear as generic
            // containers of names and values called "entities".

        using namespace EntityInterface;
        RetainedEntityInterface interf(_retainedEntities);
        Deserialize(
            formatter, interf, 
            interf.GetDocumentTypeId("GameObjects"));

        _gameObjectsCfgVal = std::make_shared<::Assets::DependencyValidation>();
        ::Assets::RegisterFileDependency(_gameObjectsCfgVal, filename);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    void EnvironmentSceneParser::PrepareFrame(
        RenderCore::IThreadContext& context, 
        SceneEngine::LightingParserContext& parserContext)
    {
        //////////////////////////////////////////////////////////// //// //// //// .... ....
            // Reload anything that might have changed on disk
        FlushLoading();
        for (const auto& p:_pimpl->_scenePlugins)
            p->LoadingPhase();

        //////////////////////////////////////////////////////////// //// //// //// .... ....
            // Culling, etc

        // RenderCore::Metal::ViewportDesc viewport(*metalContext.get());
        // auto sceneCamera = GetCameraDesc();
        // auto projectionMatrix = RenderCore::Techniques::PerspectiveProjection(
        //     sceneCamera, viewport.Width / float(viewport.Height));
        // auto worldToProjection = Combine(
        //     InvertOrthonormalTransform(sceneCamera._cameraToWorld), projectionMatrix);
        const auto& worldToProjection = parserContext.GetProjectionDesc()._worldToProjection;

        auto metalContext = RenderCore::Metal::DeviceContext::Get(context);
        _pimpl->_characters->Cull(worldToProjection);
        _pimpl->_characters->Prepare(metalContext.get());

        for (const auto&p:_pimpl->_scenePlugins)
            p->PrepareFrame(context, parserContext);
    }

    void EnvironmentSceneParser::ExecuteScene(   
        RenderCore::Metal::DeviceContext* context, 
        LightingParserContext& parserContext, 
        const SceneParseSettings& parseSettings,
        unsigned techniqueIndex) const
    {
        CPUProfileEvent pEvnt("ExecuteScene", g_cpuProfiler);

        if (    parseSettings._batchFilter == SceneParseSettings::BatchFilter::General
            ||  parseSettings._batchFilter == SceneParseSettings::BatchFilter::Depth) {

            #if defined(ENABLE_TERRAIN)
                if (parseSettings._toggles & SceneParseSettings::Toggles::Terrain) {
                    if (Tweakable("DoTerrain", true)) {
                        CPUProfileEvent pEvnt("TerrainRender", g_cpuProfiler);
                        _pimpl->_terrainManager->Render(context, parserContext, techniqueIndex);
                    }
                }
            #endif
            
            if (parseSettings._toggles & SceneParseSettings::Toggles::NonTerrain) {
                _pimpl->_characters->Render(context, parserContext, techniqueIndex);

                if (_pimpl->_placementsManager) {
                    CPUProfileEvent pEvnt("PlacementsRender", g_cpuProfiler);
                    _pimpl->_placementsManager->Render(context, parserContext, techniqueIndex);
                }
            }

        }

        for (const auto&p:_pimpl->_scenePlugins)
            p->ExecuteScene(context, parserContext, parseSettings, techniqueIndex);
    }

    void EnvironmentSceneParser::ExecuteShadowScene( 
        RenderCore::Metal::DeviceContext* context, 
        LightingParserContext& parserContext, 
        const SceneParseSettings& parseSettings,
        unsigned frustumIndex, unsigned techniqueIndex) const 
    {
        CPUProfileEvent pEvnt("ExecuteShadowScene", g_cpuProfiler);

        if (Tweakable("DoShadows", true)) {
            SceneParseSettings settings = parseSettings;
            settings._toggles &= ~SceneParseSettings::Toggles::Terrain;
            ExecuteScene(context, parserContext, settings, techniqueIndex);
        }
    }

    RenderCore::Techniques::CameraDesc EnvironmentSceneParser::GetCameraDesc() const 
    { 
        return *_pimpl->_cameraDesc;
    }

    float EnvironmentSceneParser::GetTimeValue() const      
    { 
            //  The scene parser can also provide a time value, in seconds.
            //  This is used to control rendering effects, such as wind
            //  and waves.
        return _pimpl->_time; 
    }

    void EnvironmentSceneParser::Update(float deltaTime)    
    {
        _pimpl->_characters->Update(deltaTime);
        _pimpl->_time += deltaTime; 
    }

    std::shared_ptr<PlayerCharacter>  EnvironmentSceneParser::GetPlayerCharacter()
    {
        return _pimpl->_characters->GetPlayerCharacter();
    }

    std::shared_ptr<SceneEngine::TerrainManager> EnvironmentSceneParser::GetTerrainManager()
    {
        return _pimpl->_terrainManager;
    }

    std::shared_ptr<SceneEngine::PlacementsManager> EnvironmentSceneParser::GetPlacementManager()
    {
        return _pimpl->_placementsManager;
    }

    std::shared_ptr<RenderCore::Techniques::CameraDesc> EnvironmentSceneParser::GetCameraPtr()
    {
        return _pimpl->_cameraDesc;
    }

    const PlatformRig::EnvironmentSettings& EnvironmentSceneParser::GetEnvSettings() const 
    { 
        return _pimpl->_envSettings; 
    }

    static const ::Assets::ResChar TerrainCfg[] = "terrain.cfg";
    static const ::Assets::ResChar TerrainMaterialCfg[] = "terrainmaterial.cfg";
    static const ::Assets::ResChar PlacementsCfg[] = "placements.cfg";
    static const ::Assets::ResChar EnvironmentCfg[] = "env.txt:environment";
    static const ::Assets::ResChar GameObjectsCfg[] = "gameobjects.txt";

    // static const Float3 WorldOffset(-11200.f - 7000.f, -11200.f + 700.f, 0.f);
    static const Float3 WorldOffset(-100.f, -100.f, 0.f);

    void EnvironmentSceneParser::FlushLoading()
    {
        #if defined(ENABLE_TERRAIN)
            if (!_pimpl->_terrainCfgVal || _pimpl->_terrainCfgVal->GetValidationIndex() != 0) {
                ::Assets::ConfigFileContainer<SceneEngine::TerrainConfig> container(_pimpl->MakeCfgName(TerrainCfg).c_str());
                _pimpl->_terrainManager->Load(container._asset);
                _pimpl->_terrainCfgVal = container.GetDependencyValidation();

                MainTerrainConfig = container._asset;
                MainTerrainCoords = _pimpl->_terrainManager->GetCoords();

                _pimpl->_scenePlugins.clear();
                _pimpl->_scenePlugins.push_back(
                    std::make_shared<ScenePlugin_EnvironmentFeatures>(
                        _pimpl->_cfgDir,
                        _pimpl->_retainedEntities, _pimpl->_modelCache, 
                        _pimpl->_terrainManager->GetHeightsProvider()));
            }

            if (!_pimpl->_terrainTexturesCfgVal || _pimpl->_terrainTexturesCfgVal->GetValidationIndex() != 0) {
                ::Assets::ConfigFileContainer<SceneEngine::TerrainMaterialConfig> container(_pimpl->MakeCfgName(TerrainMaterialCfg).c_str());
                _pimpl->_terrainManager->LoadMaterial(container._asset);
                _pimpl->_terrainTexturesCfgVal = container.GetDependencyValidation();
            }
        #endif

        if (!_pimpl->_placementsCfgVal || _pimpl->_placementsCfgVal->GetValidationIndex() != 0) {
            ::Assets::ConfigFileContainer<SceneEngine::WorldPlacementsConfig> container(_pimpl->MakeCfgName(PlacementsCfg).c_str());
            _pimpl->_placementsManager = std::make_shared<SceneEngine::PlacementsManager>(
                container._asset, _pimpl->_modelCache, WorldOffset);
            _pimpl->_placementsCfgVal = container.GetDependencyValidation();
        }

        if (!_pimpl->_environmentCfgVal || _pimpl->_environmentCfgVal->GetValidationIndex() != 0) {
            TRY
            {
                ::Assets::ConfigFileListContainer<PlatformRig::EnvironmentSettings> asset(_pimpl->MakeCfgName(EnvironmentCfg).c_str());
                _pimpl->_envSettings = asset._asset;
                _pimpl->_environmentCfgVal = asset.GetDependencyValidation();
            } CATCH(...) {
            } CATCH_END
        }

        if (!_pimpl->_gameObjectsCfgVal || _pimpl->_gameObjectsCfgVal->GetValidationIndex() != 0) {
            _pimpl->LoadGameObjects(_pimpl->MakeCfgName(GameObjectsCfg).c_str());
        }
    }

    EnvironmentSceneParser::EnvironmentSceneParser(const ::Assets::ResChar cfgDir[])
    {
        auto pimpl = std::make_unique<Pimpl>();
        pimpl->_time = 0.f;
        pimpl->_characters = std::make_unique<CharactersScene>();

        SplitPath<::Assets::ResChar> path(cfgDir);
        path.EndsWithSeparator() |= path.GetSectionCount() != 0;
        pimpl->_cfgDir = path.Simplify().Rebuild();

        #if defined(ENABLE_TERRAIN)
            MainTerrainFormat = std::make_shared<SceneEngine::TerrainFormat>();
            pimpl->_terrainManager = std::make_shared<SceneEngine::TerrainManager>(MainTerrainFormat);
            pimpl->_terrainManager->SetWorldSpaceOrigin(WorldOffset);
        #endif

        pimpl->_cameraDesc = std::make_shared<RenderCore::Techniques::CameraDesc>();
        if (pimpl->_characters)
            pimpl->_cameraDesc->_cameraToWorld = pimpl->_characters->DefaultCameraToWorld();
        pimpl->_cameraDesc->_nearClip = 0.5f;
        pimpl->_cameraDesc->_farClip = 6000.f;
        pimpl->_envSettings = PlatformRig::DefaultEnvironmentSettings();
        pimpl->_modelCache = std::make_shared<RenderCore::Assets::ModelCache>();

        pimpl->_retainedEntities = std::make_shared<EntityInterface::RetainedEntities>();

        _pimpl = std::move(pimpl);

        FlushLoading();
    }

    EnvironmentSceneParser::~EnvironmentSceneParser()
    {}


}

