#pragma once
#include "core/IApplicationLayer.h"
#include "panels/Panel_SceneHierarchy.h"
#include "core/event/ApplicationEvent.h"
#include <core/event/EventBus.h>

class UIApplicationLayer : public Core::IApplicationLayer
{
public:
	explicit UIApplicationLayer(Core::LayerContext& ctx);
	~UIApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

	bool OnMouseDownEvent(const Core::MouseDownEvent& e);
	bool OnMouseUpEvent(const Core::MouseUpEvent& e);
	bool OnMouseMoveEvent(const Core::MouseMoveEvent& e);
	bool OnMouseScrollEvent(const Core::MouseScrollEvent& e);

private:
	Panel_SceneHierarchy _sceneHierarchyPanel;
	Core::EventBus& _eventBus;
};