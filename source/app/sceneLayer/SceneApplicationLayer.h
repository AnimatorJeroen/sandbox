#pragma once
#include "core/IApplicationLayer.h"
#include "tests/TestScene1.h"
#include "core/event/eventBus.h"
#include <core/event/ApplicationEvent.h>


class SceneApplicationLayer : public Core::IApplicationLayer
{
	public:
	explicit SceneApplicationLayer(Core::LayerContext& ctx);
	~SceneApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;
	void OnMouseDownEvent(const Core::MouseDownEvent& e);
	private:
	Core::EventBus& _eventBus;
	TestScene1 testScene;
};