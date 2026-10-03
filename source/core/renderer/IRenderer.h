#pragma once

#include "DrawCommandBuffer.h"
#include <memory>
#include <glm/glm.hpp>

namespace Core {

    class IMesh; // Forward declaration

    class IRenderer
    {
    public:
        struct RenderTargetSpecs {
            uint32_t width  = 1280;
            uint32_t height = 720;
        };

        virtual ~IRenderer() = default;

        virtual void BeginFrame(const RenderTargetSpecs& target) = 0;

        virtual void Submit(const DrawCommandBuffer& cmdBuf) = 0;

        virtual void EndFrame() = 0;

        // Optional mesh rendering support (for renderers that support it)
        virtual std::shared_ptr<IMesh> CreateMesh() = 0;
        virtual void DrawMesh(const std::shared_ptr<IMesh>& mesh) = 0;
        
        // Optional 3D camera support (for renderers that support it)
        virtual void SetViewMatrix(const glm::mat4& view) = 0;
        virtual void SetProjectionMatrix(const glm::mat4& projection) = 0;

    };
}