// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma once

#include "EntityLayer.h"
#include "MathLayer.h"      // (for Vector3)
#include "CLIXAutoPtr.h"
#include "../EntityInterface/EntityInterface.h"
#include <memory>
#include <functional>

using namespace System::Collections::Generic;

namespace SceneEngine 
{
    class PlacementsManager; class PlacementsEditor; 
    class TerrainManager;
    class ISceneParser; class IntersectionTestScene; 
    class VegetationSpawnManager;
    class VolumetricFogManager;
    class ShallowSurfaceManager;
}
namespace Tools { class IManipulator; }

namespace EntityInterface 
{ 
    class Switch; 
    class RetainedEntities; 
    class RetainedEntityInterface; 
    class TerrainEntities; 
    class EnvEntitiesManager; 
}

namespace GUILayer
{
    ref class VisCameraSettings;
    ref class IntersectionTestContextWrapper;
	ref class IntersectionTestSceneWrapper;
    ref class PlacementsEditorWrapper;
    ref class ObjectSet;
    ref class TerrainConfig;
    class TerrainGob;
    class ObjectPlaceholders;

    class EditorScene
    {
    public:
        std::shared_ptr<SceneEngine::PlacementsManager> _placementsManager;
        std::shared_ptr<SceneEngine::PlacementsEditor> _placementsEditor;
        std::shared_ptr<SceneEngine::TerrainManager> _terrainManager;
        std::shared_ptr<SceneEngine::VegetationSpawnManager> _vegetationSpawnManager;
        std::shared_ptr<SceneEngine::VolumetricFogManager> _volumeFogManager;
        std::shared_ptr<SceneEngine::ShallowSurfaceManager> _shallowSurfaceManager;
        std::shared_ptr<EntityInterface::RetainedEntities> _flexObjects;
        std::shared_ptr<ObjectPlaceholders> _placeholders;
        std::vector<std::function<void()>> _prepareSteps;

        void    IncrementTime(float increment) { _currentTime += increment; }
        float   _currentTime;

        EditorScene();
		~EditorScene();
    };

    public ref class EditorSceneRenderSettings
    {
    public:
        System::String^ _activeEnvironmentSettings;
        ObjectSet^ _selection;
    };

    ref class IOverlaySystem;
    ref class IManipulatorSet;
    ref class IPlacementManipulatorSettingsLayer;

    /// <summary>High level manager for level editor interface</summary>
    /// The EditorSceneManager will start up and shutdown the core objects
    /// responsible for managing the scene in the level editor. This includes
    /// creating the manager objects for the EntityInterface library.
    ///
    /// This also provides a way for some level editor objects to access
    /// scene objects for export (etc)
    public ref class EditorSceneManager
    {
    public:

            //// //// ////   E X P O R T   I N T E R F A C E   //// //// ////

        value class ExportResult
        {
        public:
            System::String^ _messages;
            bool _success = false;
        };

        ref class PendingExport abstract
        {
        public:
            enum struct Type { Text, Binary, MetricsText, None };
            System::String^ _preview;
            System::String^ _messages;
            Type _previewType = Type::None;

            bool _success = false;
            
            virtual ExportResult PerformExport(System::String^ destFile) = 0;
        };

        PendingExport^ ExportEnv(EntityInterface::DocumentId docId);
        PendingExport^ ExportGameObjects(EntityInterface::DocumentId docId);
        PendingExport^ ExportPlacements(EntityInterface::DocumentId placementsDoc);

        PendingExport^ ExportTerrain(TerrainConfig^ cfg);
        PendingExport^ ExportTerrainCachedData();
        PendingExport^ ExportTerrainMaterialData();
        PendingExport^ ExportVegetationSpawn(EntityInterface::DocumentId docId);

        value class PlacementCellRef
        {
        public:
            property System::String^ NativeFile;
            property Vector3 Offset;
            property Vector3 Mins;
            property Vector3 Maxs;
        };
        PendingExport^ ExportPlacementsCfg(IEnumerable<PlacementCellRef>^ cells);

            //// //// ////   A C C E S S O R S   //// //// ////

        IManipulatorSet^ CreateTerrainManipulators();
        IManipulatorSet^ CreatePlacementManipulators(IPlacementManipulatorSettingsLayer^ context);
        IOverlaySystem^ CreateOverlaySystem(VisCameraSettings^ camera, EditorSceneRenderSettings^ renderSettings);
		IntersectionTestSceneWrapper^ GetIntersectionScene();
        PlacementsEditorWrapper^ GetPlacementsEditor();
        EntityLayer^ GetEntityInterface();

        void SetTypeAnnotation(
            uint typeId, System::String^ annotationName, 
            IEnumerable<EntityLayer::PropertyInitializer>^ initializers);

            //// //// ////   U T I L I T Y   //// //// ////

        const EntityInterface::RetainedEntities& GetFlexObjects();
        void IncrementTime(float increment);

        void UnloadTerrain();
        void ReloadTerrain(TerrainConfig^ cfg);

        EditorScene& GetScene();

            //// //// ////   C O N S T R U C T O R S   //// //// ////
        EditorSceneManager();
        ~EditorSceneManager();
        !EditorSceneManager();
    protected:
        clix::shared_ptr<EditorScene> _scene;
        clix::shared_ptr<::EntityInterface::RetainedEntityInterface> _flexGobInterface;
        clix::shared_ptr<::EntityInterface::TerrainEntities> _terrainInterface;
        clix::shared_ptr<::EntityInterface::EnvEntitiesManager> _envEntitiesManager;
        EntityLayer^ _entities;
    };
}


