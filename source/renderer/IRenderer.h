#pragma once

#include "DrawCommandBuffer.h"



class IRenderer
{
protected:
    struct RenderTargetSpecs {
        uint32_t width;
        uint32_t height;
    };

public:
    virtual void BeginFrame(const RenderTargetSpecs& target) = 0;

    virtual void Submit(const DrawCommandBuffer& cmdBuf) = 0;

    virtual void EndFrame() = 0;

};