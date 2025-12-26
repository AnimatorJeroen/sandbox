#pragma once
#include "core/IApplicationLayer.h"

class UIApplicationLayer : public Core::IApplicationLayer
{
public:
	UIApplicationLayer();
	~UIApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;
};