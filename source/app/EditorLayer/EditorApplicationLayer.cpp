#include "EditorApplicationLayer.h"
#include <imgui/imgui.h>
#include <core/event/ApplicationEvent.h>
#include <core/serializer/Serializer.h>
#include "app/sceneLayer/SceneManager.h"
#include <core/Logger.h>

EditorApplicationLayer::EditorApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx),
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
}

void EditorApplicationLayer::OnUpdate(const float deltaTime)
{
}

void EditorApplicationLayer::OnRender()
{
	_mainMenu.Render();
	_sceneHierarchyPanel.Render();
}

bool EditorApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	if(!ImGui::GetIO().WantCaptureMouse)
		return false;

	LOG_TRACE() << e.GetName() << " captured by editor layer: Button " << e.identifier;
	return true;
}

bool EditorApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	LOG_TRACE() << e.GetName() << " in editor layer: Button " << e.identifier;
	return true;
}

bool EditorApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	//LOG_TRACE() << e.GetName() << " in editor layer: " << e.posX << ", " << e.posY;
	return true;
}

bool EditorApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	//LOG_TRACE() << e.GetName() << " in editor layer: " << e.scrollX << ", " << e.scrollY;
	return true;
}

bool EditorApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	LOG_TRACE() << e.GetName() << " in editor layer: Key " << e.key << ", repeated: " << e.repeated;
	return true;
}

bool EditorApplicationLayer::OnKeyUpEvent(const Core::KeyUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	LOG_TRACE() << e.GetName() << " in editor layer: Key " << e.key;
	return true;
}

bool EditorApplicationLayer::OnKeyCharacterEvent(const Core::KeyCharacterEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;
	LOG_TRACE() << e.GetName() << " in editor layer: Character " << e.character;
	return true;
}

bool EditorApplicationLayer::OnRequestSaveSceneEvent(const RequestSaveSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	return false;
}

bool EditorApplicationLayer::OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	return false;
}

bool EditorApplicationLayer::OnChangeActiveScene(const OnChangeActiveSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	_sceneHierarchyPanel.SetContext(*_sceneManager->GetActiveScene());
	return false;
}
