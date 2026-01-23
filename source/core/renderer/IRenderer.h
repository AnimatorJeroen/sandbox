#pragma once

#include "DrawCommandBuffer.h"
#include <memory>

namespace Core {

    class IMesh; // Forward declaration

    class IRenderer
    {
    public:
        struct RenderTargetSpecs {
            uint32_t width;
            uint32_t height;
        };

        virtual ~IRenderer() = default;

        virtual void BeginFrame(const RenderTargetSpecs& target) = 0;

        virtual void Submit(const DrawCommandBuffer& cmdBuf) = 0;

        virtual void EndFrame() = 0;

        // Optional mesh rendering support (for renderers that support it)
        virtual std::shared_ptr<IMesh> CreateMesh() { return nullptr; }
        virtual void DrawMesh(const std::shared_ptr<IMesh>& mesh) { }

    };
}