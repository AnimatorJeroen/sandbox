#include "SceneApplicationLayer.h"
#include <iostream>

SceneApplicationLayer::SceneApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), testScene(*ctx.Get<Scene>().get()), _eventBus(*ctx.Get<Core::EventBus>().get())
{
	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseUpEvent, OnMouseUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseMoveEvent, OnMouseMoveEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseScrollEvent, OnMouseScrollEvent);
	testScene.Setup();
}

void SceneApplicationLayer::OnUpdate(const float deltaTime)
{
	testScene.Update(deltaTime);
}

void SceneApplicationLayer::OnRender()
{
	testScene.Render();
}

bool SceneApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	std::cout << "Mouse Button Down Event in Scene: " << e.identifier << std::endl;
	return true;
}

bool SceneApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	std::cout << "Mouse button up event in Scene: " << e.identifier << std::endl;
	return true;
}

bool SceneApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	std::cout << "Mouse move event in Scene: " << e.posX << ", " << e.posY << std::endl;
	return true;
}

bool SceneApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	std::cout << "Mouse scroll event in Scene: " << e.scrollX << ", " << e.scrollY << std::endl;
	return true;
}
