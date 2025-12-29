#include "UIApplicationLayer.h"
#include <imgui/imgui.h>
#include <core/event/ApplicationEvent.h>
#include <iostream>
#include <core/serializer/Serializer.h>
#include "app/sceneLayer/SceneManager.h"

UIApplicationLayer::UIApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx),
_sceneManager(ctx.Get<SceneManager>()),
_sceneHierarchyPanel(*_sceneManager->GetActiveScene()),
_eventBus(*ctx.Get<Core::EventBus>().get()), 
_mainMenu(*ctx.Get<Core::EventBus>().get())
{
	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseUpEvent, OnMouseUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseMoveEvent, OnMouseMoveEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseScrollEvent, OnMouseScrollEvent);

	REGISTER_CALLBACK(_eventBus, Core::KeyDownEvent, OnKeyDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyUpEvent, OnKeyUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyCharacterEvent, OnKeyCharacterEvent);

	REGISTER_CALLBACK(_eventBus, RequestSaveSceneEvent, OnRequestSaveSceneEvent);
	REGISTER_CALLBACK(_eventBus, RequestLoadSceneEvent, OnRequestLoadSceneEvent);
	REGISTER_CALLBACK(_eventBus, OnChangeActiveSceneEvent, OnChangeActiveScene);
	
	// Register callback for scene changes
	_sceneManager->RegisterSceneChangedCallback([this](std::shared_ptr<Scene> newScene) {
		_sceneHierarchyPanel.SetContext(*newScene);
	});
}

void UIApplicationLayer::OnUpdate(const float deltaTime)
{
}

void UIApplicationLayer::OnRender()
{
	_mainMenu.Render();
	_sceneHierarchyPanel.Render();
}

bool UIApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	if(!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse Button Down Event captured by UI Layer: " << e.identifier << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse button up event in UI: " << e.identifier << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse move event in UI: " << e.posX << ", " << e.posY << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse scroll event in UI: " << e.scrollX << ", " << e.scrollY << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	std::cout << "Key down event in UI: " << e.key << ", repeated: " << e.repeated << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyUpEvent(const Core::KeyUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	std::cout << "Key up event in UI: " << e.key << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyCharacterEvent(const Core::KeyCharacterEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;
	std::cout << "Key character event in UI: " << e.character << std::endl;
	return true;
}

bool UIApplicationLayer::OnRequestSaveSceneEvent(const RequestSaveSceneEvent& e)
{
	std::cout << "Editor request to save scene received." << std::endl;
	return false;
}

bool UIApplicationLayer::OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e)
{
	std::cout << "Editor request to load scene received." << std::endl;
	return false;
}

bool UIApplicationLayer::OnChangeActiveScene(const OnChangeActiveSceneEvent& e)
{
	std::cout << "Scene reloaded event received in UI layer." << std::endl;
	_sceneHierarchyPanel.SetContext(*_sceneManager->GetActiveScene());
	return false;
}
