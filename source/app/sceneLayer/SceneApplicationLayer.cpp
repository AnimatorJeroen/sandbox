#include "SceneApplicationLayer.h"
#include <iostream>
#include <core/serializer/Serializer.h>

SceneApplicationLayer::SceneApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), 
_scene(ctx.Get<Scene>()), 
_testScene(*ctx.Get<Scene>().get()), 
_eventBus(*ctx.Get<Core::EventBus>().get())
{
	_testScene.Setup();
	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseUpEvent, OnMouseUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseMoveEvent, OnMouseMoveEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseScrollEvent, OnMouseScrollEvent);

	REGISTER_CALLBACK(_eventBus, Core::KeyDownEvent, OnKeyDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyUpEvent, OnKeyUpEvent);

	REGISTER_CALLBACK(_eventBus, Core::WindowResizeEvent, OnWindowResizedEvent);

	REGISTER_CALLBACK(_eventBus, EditorRequestSaveSceneEvent, OnEditorRequestSaveSceneEvent);
	REGISTER_CALLBACK(_eventBus, EditorRequestLoadSceneEvent, OnEditorRequestLoadSceneEvent);
}

void SceneApplicationLayer::OnUpdate(const float deltaTime)
{
	_testScene.Update(deltaTime);
}

void SceneApplicationLayer::OnRender()
{
	_testScene.Render();
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

bool SceneApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	std::cout << "Key down event in Scene: " << e.key << ", repeated: " << e.repeated << std::endl;
	return true;
}

bool SceneApplicationLayer::OnKeyUpEvent(const Core::KeyUpEvent& e)
{
	std::cout << "Key up event in Scene: " << e.key << std::endl;
	return true;
}

bool SceneApplicationLayer::OnWindowResizedEvent(const Core::WindowResizeEvent& e)
{
	std::cout << "Window resized in Scene Layer to: " << e.Width << "x" << e.Height << std::endl;
	auto specs = _testScene.GetRenderSpecs();
	specs.height = e.Height;
	specs.width = e.Width;
	_testScene.SetRenderSpecs(specs);
	return false;
}

bool SceneApplicationLayer::OnEditorRequestSaveSceneEvent(const EditorRequestSaveSceneEvent& e)
{
	std::cout << "save scene received in scene layer." << std::endl;

	const std::string sceneFilePath = "saved files/scene.dat";
	bool success = Core::Serializer::Serialize<Scene>(_scene, sceneFilePath);
	return true;
}

bool SceneApplicationLayer::OnEditorRequestLoadSceneEvent(const EditorRequestLoadSceneEvent& e)
{
	std::cout << "load scene received in scene layer." << std::endl;
	const std::string sceneFilePath = "saved files/scene.dat";
	_scene = Core::Serializer::Deserialize<Scene>(sceneFilePath);
	return true;
}
