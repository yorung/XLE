// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma once

#include "../../../RenderCore/IThreadContext_Forward.h"

namespace RenderCore { namespace Techniques { class ProjectionDesc; class ParsingContext; } }
namespace SceneEngine { class LightingParserContext; }

namespace XLEBridgeUtils
{
    public delegate void RenderCallback(LevelEditorCore::DesignView^ designView, Sce::Atf::Rendering::Camera^ camera);

    private ref class ManipulatorOverlay : public GUILayer::IOverlaySystem
    {
    public:
        virtual void RenderToScene(
            RenderCore::IThreadContext* device, 
            SceneEngine::LightingParserContext& parserContext) override;

        virtual void RenderWidgets(
            RenderCore::IThreadContext* device, 
            const RenderCore::Techniques::ProjectionDesc& projectionDesc) override;
        virtual void SetActivationState(bool) override;

        event RenderCallback^ OnRender;

        ManipulatorOverlay(
            LevelEditorCore::DesignView^ designView,
            LevelEditorCore::ViewControl^ viewControl);
        ~ManipulatorOverlay();
        !ManipulatorOverlay();

        static SceneEngine::LightingParserContext* s_currentParsingContext = nullptr;
    protected:
        LevelEditorCore::DesignView^ _designView;
        LevelEditorCore::ViewControl^ _viewControl;
    };
}

