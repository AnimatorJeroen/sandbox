#include "SceneApplicationLayer.h"
#include "SceneManager.h"
#include <core/serializer/Serializer.h>
#include <core/Logger.h>

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
	LOG_TRACE(std::string(e.GetName()) + " in Scene: Button " + std::to_string(e.identifier));
	return false;
}

bool SceneApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " in Scene: Button " + std::to_string(e.identifier));
	return false;
}

bool SceneApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	return false;
}

bool SceneApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	//LOG_TRACE(std::string(e.GetName()) + " in Scene: " + std::to_string(e.scrollX) + ", " + std::to_string(e.scrollY));
	return false;
}

bool SceneApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " in Scene: Key " + std::to_string(e.key) + ", repeated: " + std::to_string(e.repeated));
	return false;
}

bool SceneApplicationLayer::OnKeyUpEvent(const Core::KeyUpEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " in Scene: Key " + std::to_string(e.key));
	return false;
}

bool SceneApplicationLayer::OnWindowResizedEvent(const Core::WindowResizeEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " in Scene Layer: " + std::to_string(e.Width) + "x" + std::to_string(e.Height));
	auto specs = _testScene.GetRenderSpecs();
	specs.height = e.Height;
	specs.width = e.Width;
	_testScene.SetRenderSpecs(specs);
	return false;
}

bool SceneApplicationLayer::OnRequestSaveSceneEvent(const RequestSaveSceneEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " received in scene layer.");

	const std::string sceneFilePath = "saved files/scene.dat";
	bool success = _sceneManager->SaveActiveScene(sceneFilePath);
	
	return true;
}

bool SceneApplicationLayer::OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " received in scene layer.");
	const std::string sceneFilePath = "saved files/scene.dat";
	
	// Use SceneManager to load - it handles everything and switches the active scene
	_sceneManager->LoadScene(sceneFilePath, true);
	return true;
}

bool SceneApplicationLayer::OnChangeActiveScene(const OnChangeActiveSceneEvent& e)
{
	LOG_TRACE(std::string(e.GetName()) + " received in scene layer.");
	return false; // Let other layers handle it too
}
