#pragma once

#include "DrawCommandBuffer.h"


namespace Core {

    class IRenderer
    {
    public:
        struct RenderTargetSpecs {
            uint32_t width;
            uint32_t height;
        };

        virtual void BeginFrame(const RenderTargetSpecs& target) = 0;

        virtual void Submit(const DrawCommandBuffer& cmdBuf) = 0;

        virtual void EndFrame() = 0;

    };
}