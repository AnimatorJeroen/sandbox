#pragma once
#include "core/IApplicationLayer.h"
#include "tests/TestScene1.h"

class SceneApplicationLayer : public Core::IApplicationLayer
{
	public:
	explicit SceneApplicationLayer(Core::LayerContext& ctx);
	~SceneApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

private:
	TestScene1 testScene;
};