#pragma once
#include "core/IApplicationLayer.h"
#include "Panel_SceneHierarchy.h"

class UIApplicationLayer : public Core::IApplicationLayer
{
public:
	explicit UIApplicationLayer(Core::LayerContext& ctx);
	~UIApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

private:
	Panel_SceneHierarchy m_sceneHierarchyPanel;
};