#pragma once

#include "IRenderer.h"
#include <memory>
#include <glm/glm.hpp>

namespace Core {

    class Renderer_ImGui : public IRenderer
    {
    public:
        void BeginFrame(const IRenderer::RenderTargetSpecs& target) override;

        void Submit(const DrawCommandBuffer& cmdBuf) override;

        void EndFrame() override;

        // Mesh rendering not supported in ImGui
        std::shared_ptr<IMesh> CreateMesh() override;
        void DrawMesh(const std::shared_ptr<IMesh>& mesh) override;
        
        // 3D camera not supported in ImGui (2D only)
        void SetViewMatrix(const glm::mat4& view) override;
        void SetProjectionMatrix(const glm::mat4& projection) override;

    };
}