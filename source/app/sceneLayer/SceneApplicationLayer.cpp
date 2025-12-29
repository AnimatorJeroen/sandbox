#include "SceneApplicationLayer.h"
#include "SceneManager.h"
#include <iostream>
#include <core/serializer/Serializer.h>

SceneApplicationLayer::SceneApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx),
_sceneManager(ctx.Get<SceneManager>()),
_testScene(ctx.Get<SceneManager>()),
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

	REGISTER_CALLBACK(_eventBus, RequestSaveSceneEvent, OnRequestSaveSceneEvent);
	REGISTER_CALLBACK(_eventBus, RequestLoadSceneEvent, OnRequestLoadSceneEvent);
	REGISTER_CALLBACK(_eventBus, OnChangeActiveSceneEvent, OnChangeActiveScene);
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

bool SceneApplicationLayer::OnRequestSaveSceneEvent(const RequestSaveSceneEvent& e)
{
	std::cout << "save scene received in scene layer." << std::endl;

	const std::string sceneFilePath = "saved files/scene.dat";
	bool success = _sceneManager->SaveActiveScene(sceneFilePath);
	
	return true;
}

bool SceneApplicationLayer::OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e)
{
	std::cout << "load scene received in scene layer." << std::endl;
	const std::string sceneFilePath = "saved files/scene.dat";
	
	// Use SceneManager to load - it handles everything and switches the active scene
	auto newScene = _sceneManager->LoadScene(sceneFilePath, true);
	
	if (newScene) {
		// Notify other layers that scene has been reloaded
		_eventBus.PushEvent<OnChangeActiveSceneEvent>(OnChangeActiveSceneEvent());
	}
	
	return true;
}

bool SceneApplicationLayer::OnChangeActiveScene(const OnChangeActiveSceneEvent& e)
{
	std::cout << "Scene reloaded event received in scene layer." << std::endl;
	return false; // Let other layers handle it too
}
