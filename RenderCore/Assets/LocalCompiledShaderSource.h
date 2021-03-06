// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma once

#include "../ShaderService.h"
#include "../../Assets/IntermediateAssets.h"
#include "../../Utility/Threading/ThreadingUtils.h"
#include <vector>
#include <memory>

namespace RenderCore { namespace Assets 
{
    class ShaderCacheSet;
    class ShaderCompileMarker;

    class LocalCompiledShaderSource 
        : public ::Assets::IntermediateAssets::IAssetCompiler
        , public ShaderService::IShaderSource
        , public std::enable_shared_from_this<LocalCompiledShaderSource>
    {
    public:
        std::shared_ptr<::Assets::PendingCompileMarker> PrepareAsset(
            uint64 typeCode, const ::Assets::ResChar* initializers[], unsigned initializerCount,
            const ::Assets::IntermediateAssets::Store& destinationStore);

        using IPendingMarker = ShaderService::IPendingMarker;
        std::shared_ptr<IPendingMarker> CompileFromFile(
            const ShaderService::ResId& resId, 
            const ::Assets::ResChar definesTable[]) const;
            
        std::shared_ptr<IPendingMarker> CompileFromMemory(
            const char shaderInMemory[], const char entryPoint[], 
            const char shaderModel[], const ::Assets::ResChar definesTable[]) const;

        void StallOnPendingOperations(bool cancelAll);

        ShaderCacheSet& GetCacheSet() { return *_shaderCacheSet; }

        LocalCompiledShaderSource();
        ~LocalCompiledShaderSource();
    protected:
        std::unique_ptr<ShaderCacheSet> _shaderCacheSet;
        std::vector<std::shared_ptr<ShaderCompileMarker>> _activeCompileOperations;
        mutable Interlocked::Value _activeCompileCount;
    };
}}

