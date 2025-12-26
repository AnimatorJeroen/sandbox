#pragma once

#include "IRenderer.h"

namespace Core {

    class Renderer_ImGui : public IRenderer
    {
    public:
        void BeginFrame(const IRenderer::RenderTargetSpecs& target);

        void Submit(const DrawCommandBuffer& cmdBuf);

        void EndFrame();

    };
}