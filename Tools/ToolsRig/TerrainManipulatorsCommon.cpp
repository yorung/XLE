// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "TerrainManipulatorsCommon.h"
#include "ManipulatorsUtil.h"
#include "ManipulatorsRender.h"
#include "../../RenderOverlays/DebuggingDisplay.h"
#include "../../SceneEngine/SceneEngineUtils.h"
#include "../../SceneEngine/IntersectionTest.h"
#include "../../SceneEngine/LightingParserContext.h"
#include "../../SceneEngine/Terrain.h"
#include "../../SceneEngine/TerrainConfig.h"
#include "../../Math/Transformations.h"
#include "../../Utility/TimeUtils.h"

static unsigned FrameRenderCount;

namespace ToolsRig
{
    using SceneEngine::IntersectionTestContext;
    using SceneEngine::IntersectionTestScene;
    std::pair<Float3, bool> FindTerrainIntersection(
        const IntersectionTestContext& context, const IntersectionTestScene& scene,
        const Int2 screenCoords)
    {
        auto result = scene.UnderCursor(context, screenCoords, IntersectionTestScene::Type::Terrain);
        if (result._type == IntersectionTestScene::Type::Terrain) {
            return std::make_pair(result._worldSpaceCollision, true);
        }
        return std::make_pair(Float3(0.f, 0.f, 0.f), false);
    }

    IManipulator::~IManipulator() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

    TerrainManipulatorBase::TerrainManipulatorBase(std::shared_ptr<SceneEngine::TerrainManager> terrainManager)
        : _terrainManager(terrainManager)
    {}

    Float2 TerrainManipulatorBase::WorldSpaceToTerrain(const Float2& input) const
    {
        return WorldSpaceToCoverage(SceneEngine::CoverageId_Heights, input);
    }

    float TerrainManipulatorBase::WorldSpaceDistanceToTerrainCoords(float input) const
    {
        return WorldSpaceToCoverageDistance(SceneEngine::CoverageId_Heights, input);
    }

    Float2 TerrainManipulatorBase::TerrainToWorldSpace(const Float2& input) const
    {
        auto& coords = _terrainManager->GetCoords();
        auto& cfg = _terrainManager->GetConfig();
        auto coverToCell = Inverse(AsFloat4x4(Float2x3(cfg.CellBasedToCoverage(SceneEngine::CoverageId_Heights))));
        auto cellToWorld = coords.CellBasedToWorld();
        return Truncate(TransformPoint(Combine(coverToCell, cellToWorld), Expand(input, 0.f)));
    }

    Float2 TerrainManipulatorBase::WorldSpaceToCoverage(unsigned layerId, const Float2& input) const
    {
            // This should only require a 2D scale and translation
            //  ... but it's got much more expensive with the new terrain config interface
            //      there's a lot of redundant maths involved
            //      Still, even though it's expensive, it's better with the simple interface
        auto& coords = _terrainManager->GetCoords();
        auto& cfg = _terrainManager->GetConfig();
        auto cellToCover = AsFloat4x4(Float2x3(cfg.CellBasedToCoverage(layerId)));
        auto worldToCell = coords.WorldToCellBased();
        return Truncate(TransformPoint(Combine(worldToCell, cellToCover), Expand(input, 0.f)));
    }

    float TerrainManipulatorBase::WorldSpaceToCoverageDistance(unsigned layerId, float input) const
    {
            // huge amount of redundant math involved here; to do just a simple thing
            // but, even still, it should be ok...?
        auto& coords = _terrainManager->GetCoords();
        auto& cfg = _terrainManager->GetConfig();
        auto cellToCover = AsFloat4x4(Float2x3(cfg.CellBasedToCoverage(layerId)));
        auto worldToConver = Combine(coords.WorldToCellBased(), cellToCover);
        float scale = .5f * (worldToConver(0,0) + worldToConver(1,1));
        return input * scale;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    bool    CommonManipulator::OnInputEvent(
        const RenderOverlays::DebuggingDisplay::InputSnapshot& evnt, 
        const SceneEngine::IntersectionTestContext& hitTestContext,
        const SceneEngine::IntersectionTestScene& hitTestScene)
    {
        const bool shiftHeld = evnt.IsHeld(RenderOverlays::DebuggingDisplay::KeyId_Make("shift"));
        if (evnt._wheelDelta) {
                // on wheel delta, change effect size
            if (shiftHeld) {
                _strength = std::max(0.f, _strength + 3.f * evnt._wheelDelta / 120.f);
            } else {
                _size = std::max(1.f, _size + 3.f * evnt._wheelDelta / 120.f);
            }
        }

            //  We can do the terrain intersection test now, but it requires setting up 
            //  new device and lighting parser contexts. We need to know the viewport -- and the only way 
            //  to do that is to get it from the windows HWND!
        Int2 newMouseCoords(evnt._mousePosition[0], evnt._mousePosition[1]);
            // only do the terrain test if we get some kind of movement
        if (((XlAbs(_mouseCoords[0] - newMouseCoords[0]) > 1 || XlAbs(_mouseCoords[1] - newMouseCoords[1]) > 1)
            && (FrameRenderCount > _lastRenderCount0)) || evnt.IsPress_LButton()) {

            _currentWorldSpaceTarget = FindTerrainIntersection(hitTestContext, hitTestScene, newMouseCoords);
            _lastPerform = 0;
            _mouseCoords = newMouseCoords;
            _lastRenderCount0 = FrameRenderCount;

            if (evnt.IsPress_LButton()) {
                _targetOnMouseDown = _currentWorldSpaceTarget;
            }
        }

        if (evnt.IsHeld_LButton()) {
                // perform action -- (like raising or lowering the terrain)
            if (_currentWorldSpaceTarget.second && (Millisecond_Now() - _lastPerform) > 33 && (FrameRenderCount > _lastRenderCount1)) {

                TRY {
                    PerformAction(_currentWorldSpaceTarget.first, _size, shiftHeld?(-_strength):_strength);
                } CATCH (...) {
                } CATCH_END
                
                _lastPerform = Millisecond_Now();
                _lastRenderCount1 = FrameRenderCount;
            }
            return true;
        }
        return false;
    }

    void    CommonManipulator::Render(
                    RenderCore::IThreadContext* context, 
                    SceneEngine::LightingParserContext& parserContext)
    {
            //  Draw a highlight on the area that we're going to modify. Since we want this to behave like a decal, 
            //  it's best to do this rendering after the gbuffer is fully prepared, and we should render onto the
            //  lighting buffer.
            //  In theory, we could stencil out the terrain, as well -- so we only actually draw onto terrain 
            //  geometry.
        if (_currentWorldSpaceTarget.second) {
            RenderCylinderHighlight(
                *context, parserContext, _currentWorldSpaceTarget.first, _size);
        }
		++FrameRenderCount;
    }

    CommonManipulator::CommonManipulator(std::shared_ptr<SceneEngine::TerrainManager> terrainManager)
        : TerrainManipulatorBase(std::move(terrainManager))
    {
        _currentWorldSpaceTarget = std::make_pair(Float3(0,0,0), false);
        _targetOnMouseDown = std::make_pair(Float3(0,0,0), false);
        _mouseCoords = Int2(0,0);
        _strength = 1.f;
        _size = 20.f;
        _lastPerform = 0;
        _lastRenderCount0 = _lastRenderCount1 = 0;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////

    bool    RectangleManipulator::OnInputEvent(
        const RenderOverlays::DebuggingDisplay::InputSnapshot& evnt, 
        const IntersectionTestContext& hitTestContext,
        const IntersectionTestScene& hitTestScene)
    {
        Int2 mousePosition(evnt._mousePosition[0], evnt._mousePosition[1]);

        if (evnt.IsPress_LButton()) {
                // on lbutton press, we should place a new anchor
            auto intersection = FindTerrainIntersection(hitTestContext, hitTestScene, mousePosition);
            _isDragging = intersection.second;
            if (intersection.second) {
                _firstAnchor = intersection.first;
                _secondAnchor = intersection;
            }
        }

        if (_isDragging) {

            if (evnt.IsHeld_LButton() || evnt.IsRelease_LButton()) {
                    // update the second anchor as we drag
                _secondAnchor = FindTerrainIntersection(hitTestContext, hitTestScene, mousePosition);
            }

            if (evnt.IsRelease_LButton()) {
                    // on release, we should perform the action
                    //  (assuming we released at a valid position)
                _isDragging = false;

                if (_secondAnchor.second) {

                    Float2 faTerrain = WorldSpaceToTerrain(Truncate(_firstAnchor));
                    Float2 fsTerrain = WorldSpaceToTerrain(Truncate(_secondAnchor.first));
                    Float2 terrainCoordsMins(std::min(faTerrain[0], fsTerrain[0]), std::min(faTerrain[1], fsTerrain[1]));
                    Float2 terrainCoordsMaxs(std::max(faTerrain[0], fsTerrain[0]), std::max(faTerrain[1], fsTerrain[1]));
                    Float2 faWorld = TerrainToWorldSpace(RoundDownToInteger(terrainCoordsMins));
                    Float2 fsWorld = TerrainToWorldSpace(RoundUpToInteger(terrainCoordsMaxs));

                    TRY {
                        PerformAction(Expand(faWorld, 0.f), Expand(fsWorld, 0.f));
                    } CATCH(...) {
                    } CATCH_END
                }
            }

        }

        return false;
    }

    void    RectangleManipulator::Render(RenderCore::IThreadContext* context, SceneEngine::LightingParserContext& parserContext)
    {
            //  while dragging, we should draw a rectangle highlight on the terrain
        using namespace RenderCore::Metal;
        if (_isDragging && _secondAnchor.second) {

                // clamp anchor values to the terrain coords size
            Float2 faTerrain = WorldSpaceToTerrain(Truncate(_firstAnchor));
            Float2 fsTerrain = WorldSpaceToTerrain(Truncate(_secondAnchor.first));
            Float2 terrainCoordsMins(std::min(faTerrain[0], fsTerrain[0]), std::min(faTerrain[1], fsTerrain[1]));
            Float2 terrainCoordsMaxs(std::max(faTerrain[0], fsTerrain[0]), std::max(faTerrain[1], fsTerrain[1]));
            Float2 faWorld = TerrainToWorldSpace(RoundDownToInteger(terrainCoordsMins));
            Float2 fsWorld = TerrainToWorldSpace(RoundUpToInteger(terrainCoordsMaxs));

            RenderRectangleHighlight(
                *context, parserContext,
                Float3(std::min(faWorld[0], fsWorld[0]), std::min(faWorld[1], fsWorld[1]), 0.f),
                Float3(std::max(faWorld[0], fsWorld[0]), std::max(faWorld[1], fsWorld[1]), 0.f));
        }
    }

    RectangleManipulator::RectangleManipulator(std::shared_ptr<SceneEngine::TerrainManager> terrainManager)
        : TerrainManipulatorBase(terrainManager)
    {
        _isDragging = false;
    }

}

