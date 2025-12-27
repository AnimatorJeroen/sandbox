#include "SceneApplicationLayer.h"
#include <iostream>

SceneApplicationLayer::SceneApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), testScene(*ctx.Get<Scene>().get()), _eventBus(*ctx.Get<Core::EventBus>().get())
{
	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
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

void SceneApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	std::cout << "Mouse Button Down Event in Scene: " << e.identifier << std::endl;
}
