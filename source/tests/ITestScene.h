#pragma once
#include "core/renderer/IRenderer.h"

class ITestScene
{
public:
	virtual void Setup() = 0;
	virtual void Teardown() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual void SetRenderSpecs(const Core::IRenderer::RenderTargetSpecs& specs) = 0;
	virtual const Core::IRenderer::RenderTargetSpecs& GetRenderSpecs() const = 0;
	virtual ~ITestScene() = default;
};