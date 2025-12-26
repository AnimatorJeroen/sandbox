#pragma once
#include "core/IApplicationLayer.h"

class SceneApplicationLayer : public Core::IApplicationLayer
{
	public:
	SceneApplicationLayer();
	~SceneApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;
};