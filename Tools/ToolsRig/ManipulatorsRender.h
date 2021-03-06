// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma once

#include "../../RenderCore/IThreadContext_Forward.h"
#include "../../RenderCore/Metal/Forward.h"
#include "../../Math/Vector.h"
#include "../../Core/Types.h"
#include <utility>

namespace RenderCore { namespace Techniques { class ParsingContext; }}
namespace SceneEngine 
{ 
    class PlacementsEditor; 
    typedef std::pair<uint64, uint64> PlacementGUID;
}

namespace ToolsRig
{
    void Placements_RenderHighlight(
        RenderCore::IThreadContext& threadContext,
        RenderCore::Techniques::ParsingContext& parserContext,
        SceneEngine::PlacementsEditor* editor,
        const SceneEngine::PlacementGUID* filterBegin,
        const SceneEngine::PlacementGUID* filterEnd,
        uint64 materialGuid = ~0ull);

    void Placements_RenderFiltered(
        RenderCore::Metal::DeviceContext& metalContext,
        RenderCore::Techniques::ParsingContext& parserContext,
        SceneEngine::PlacementsEditor* editor,
        const SceneEngine::PlacementGUID* filterBegin,
        const SceneEngine::PlacementGUID* filterEnd,
        uint64 materialGuid = ~0ull);

    void RenderCylinderHighlight(
        RenderCore::IThreadContext& threadContext, 
        RenderCore::Techniques::ParsingContext& parserContext,
        const Float3& centre, float radius);

    void RenderRectangleHighlight(
        RenderCore::IThreadContext& threadContext, 
        RenderCore::Techniques::ParsingContext& parserContext,
        const Float3& mins, const Float3& maxs);

    void DrawQuadDirect(
        RenderCore::IThreadContext& threadContext, 
        const RenderCore::Metal::ShaderResourceView& srv, 
        Float2 screenMins, Float2 screenMaxs);
}

