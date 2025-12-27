#pragma once
#include "core/IApplicationLayer.h"
#include "tests/TestScene1.h"
#include "core/event/eventBus.h"
#include <core/event/MouseEvent.h>
#include <core/event/KeyEvent.h>


class SceneApplicationLayer : public Core::IApplicationLayer
{
	public:
	explicit SceneApplicationLayer(Core::LayerContext& ctx);
	~SceneApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

	bool OnMouseDownEvent(const Core::MouseDownEvent& e);
	bool OnMouseUpEvent(const Core::MouseUpEvent& e);
	bool OnMouseMoveEvent(const Core::MouseMoveEvent& e);
	bool OnMouseScrollEvent(const Core::MouseScrollEvent& e);

	bool OnKeyDownEvent(const Core::KeyDownEvent& e);
	bool OnKeyUpEvent(const Core::KeyUpEvent& e);

	private:
	Core::EventBus& _eventBus;
	TestScene1 testScene;
};