#include "pch.h"
#include "EditorApplicationLayer.h"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <core/serializer/Serializer.h>
#include "app/sceneLayer/SceneManager.h"
#include "app/sceneLayer/components/Components.h"
#include <core/event/EventBus.h>
#include <core/Logger.h>
#include <core/Window.h>
#include <core/BrowserWindow.h>

#include <random>

EditorApplicationLayer::EditorApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx),
//services
_windowHandle(ctx.Get<Core::Window>()->GetNativeWindowHandle()),
_sceneManager(ctx.Get<SceneManager>()),
_eventBus(*ctx.Get<Core::EventBus>().get()), 
_undoManager(),
_applicator(_undoManager),
//editor context
_editorContext(*_sceneManager, _eventBus, _applicator, _undoManager, _windowHandle),
//ui panels
_mainMenu(_eventBus, _editorContext),
_sceneHierarchyPanel(*_sceneManager->GetActiveScene(), _editorContext),
_openDocumentsTopBar(*_sceneManager, *ctx.Get<Core::EventBus>().get(), _editorContext),
_propertiesBar(_editorContext),
//popup system
_popupManager(),
_cameraController()
{
	// Set popup manager reference in editor context
	_editorContext.SetPopupManager(&_popupManager);
	
	// Set window reference for blocking popups
	_popupManager.SetWindow(ctx.Get<Core::Window>().get());

	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseUpEvent, OnMouseUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseMoveEvent, OnMouseMoveEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseScrollEvent, OnMouseScrollEvent);

	REGISTER_CALLBACK(_eventBus, Core::KeyDownEvent, OnKeyDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyUpEvent, OnKeyUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyCharacterEvent, OnKeyCharacterEvent);

	REGISTER_CALLBACK(_eventBus, RequestLoadSceneEvent, OnRequestLoadSceneEvent);
	REGISTER_CALLBACK(_eventBus, RequestCloseSceneEvent, OnRequestCloseSceneEvent);
	REGISTER_CALLBACK(_eventBus, OnChangeActiveSceneEvent, OnChangeActiveScene);
	REGISTER_CALLBACK(_eventBus, RequestUndoEvent, OnRequestUndo);
	REGISTER_CALLBACK(_eventBus, RequestRedoEvent, OnRequestRedo);

	REGISTER_CALLBACK(_eventBus, Core::ApplicationCloseEvent, OnApplicationCloseEvent);
	REGISTER_CALLBACK(_eventBus, Core::RequestApplicationCloseEvent, OnRequestApplicationCloseEvent);

	REGISTER_CALLBACK((_eventBus), OnDestroySceneEvent, OnDestroyScene);
}

void EditorApplicationLayer::OnUpdate(const float deltaTime)
{
}

void EditorApplicationLayer::OnRender()
{
	_mainMenu.Render();
	_openDocumentsTopBar.Render();
	_propertiesBar.Render();
	_sceneHierarchyPanel.Render();
	
	RenderImGuizmo();

	// Render popups last so they appear on top of everything
	_popupManager.Render();
}



bool EditorApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	if (e.identifier == 0)
		_isLeftMouseDown = true;
	else if (e.identifier == 1)
		_isRightMouseDown = true;
	else if (e.identifier == 2)
		_isMiddleMouseDown = true;

	_cameraController.OnMouseDown(_lastMouseX, _lastMouseY);

	if(!ImGui::GetIO().WantCaptureMouse)
		return false;

	LOG_TRACE() << e.GetName() << " captured by editor layer: Button " << e.identifier;
	return true;
}

bool EditorApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	if (e.identifier == 0)
		_isLeftMouseDown = false;
	if (e.identifier == 1)
		_isRightMouseDown = false;
	if (e.identifier == 2)
		_isMiddleMouseDown = false;

	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	LOG_TRACE() << e.GetName() << " in editor layer: Button " << e.identifier;
	return true;
}

bool EditorApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	_lastMouseX = e.posX;
	_lastMouseY = e.posY;

	if (!ImGui::GetIO().WantCaptureMouse)
		_cameraController.OnMouseMove(e.posX, e.posY, _isLeftMouseDown, _isMiddleMouseDown, _isRightMouseDown);

	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	//LOG_TRACE() << e.GetName() << " in editor layer: " << e.posX << ", " << e.posY;
	return true;
}

bool EditorApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	_cameraController.OnMouseScroll(e.scrollY);

	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	//LOG_TRACE() << e.GetName() << " in editor layer: " << e.scrollX << ", " << e.scrollY;
	return true;
}

bool EditorApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	//if (!ImGui::GetIO().WantCaptureKeyboard)
	//	return false;

	LOG_TRACE() << e.GetName() << " in editor layer: Key " << e.key << ", repeated: " << e.repeated;

	// Check modifiers directly from the event
	if (e.key == 'Z') {
		if ((e.mods & Core::KMOD_CONTROL) && (e.mods & Core::KMOD_SHIFT)) {
			// Ctrl+Shift+Z: Redo
			_eventBus.PushEvent(RequestRedoEvent());
		}
		else if (e.mods & Core::KMOD_CONTROL) {
			// Ctrl+Z: Undo
			_eventBus.PushEvent(RequestUndoEvent());
		}
	}
	else if (e.key == 'S' && !e.repeated) {
		if ((e.mods & Core::KMOD_CONTROL) && (e.mods & Core::KMOD_SHIFT))
		{
			auto activeSceneIndex = _sceneManager->GetActiveSceneIndex();
			_editorContext.SaveSceneAs(activeSceneIndex);
		}
		else if (e.mods & Core::KMOD_CONTROL)
		{
			auto activeSceneIndex = _sceneManager->GetActiveSceneIndex();
			_editorContext.SaveScene(activeSceneIndex);
		}
	}
	else if (e.key == 'N' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.NewScene("Untitled", true);
		}
	}
	else if (e.key == 'O' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.OpenScene();
		}
	}
	else if (e.key == 'A' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			auto scene = _sceneManager->GetActiveScene();
			if (scene)
			{
				auto& registry = scene->GetRegistry();
				auto view = registry.view<Core::UUID>();

				std::set<entt::entity> entities(view.begin(), view.end());
				if(entities.find(scene->GetSceneEntity()) != entities.end())
				{
					entities.erase(scene->GetSceneEntity());
				}
				_editorContext.SetSelection(entities);
			}
		}
	}
	else if (e.key == 'C' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.Copy();
		}
	}
	else if (e.key == 'X' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.Cut();
		}
	}
	else if (e.key == 'V' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.Paste();
		}
	}
	else if (e.key == 'W' && !e.repeated) {
		_editorContext.SetImGuizmoOperation(ImGuizmo::TRANSLATE);
	}
	else if (e.key == 'E' && !e.repeated) {
		_editorContext.SetImGuizmoOperation(ImGuizmo::ROTATE);
	}
	else if (e.key == 'R' && !e.repeated) {
		_editorContext.SetImGuizmoOperation(ImGuizmo::SCALE);
	}

	

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

bool EditorApplicationLayer::OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	return false;
}

bool EditorApplicationLayer::OnApplicationCloseEvent(const Core::ApplicationCloseEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	return false;
}

bool EditorApplicationLayer::OnRequestApplicationCloseEvent(const Core::RequestApplicationCloseEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";

	while (_sceneManager->GetSceneCount() > 0)
	{
		int sceneIndex = _sceneManager->GetSceneCount() - 1;

		bool isDirty = _editorContext.IsSceneDirty(sceneIndex);
		if (!isDirty)
		{
			_editorContext.CloseScene(sceneIndex);
			continue;
		}
		PopupResult result = InvokePopupRequestSaveChanges(sceneIndex);

		if (result == PopupResult::Yes)
		{
			LOG_DEBUG() << "Saving and closing scene...";
			_editorContext.SaveScene(sceneIndex);
			_editorContext.CloseScene(sceneIndex);
		}
		else if (result == PopupResult::No)
		{
			LOG_DEBUG() << "Closing scene without saving...";
			_editorContext.CloseScene(sceneIndex);
		}
		else // Cancel
		{
			LOG_DEBUG() << "Application close cancelled by user.";
			return true; // Cancel application close
		}
	}
	return false;
}

bool EditorApplicationLayer::OnRequestCloseSceneEvent(const RequestCloseSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";

	bool isDirty = _editorContext.IsSceneDirty(e.sceneIndex);
	if (!isDirty)
	{
		_editorContext.CloseScene(e.sceneIndex);
		return true;
	}
	// Show blocking confirmation
	PopupResult result = InvokePopupRequestSaveChanges(e.sceneIndex);
	
	if (result == PopupResult::Yes)
	{
		// TODO: Save the scene before closing
		LOG_DEBUG() << "Saving and closing scene: " << e.sceneIndex;
		_editorContext.SaveScene(e.sceneIndex);
		auto scene = _sceneManager->GetScene(e.sceneIndex);
		_editorContext.CloseScene(e.sceneIndex);
	}
	else if (result == PopupResult::No)
	{
		LOG_DEBUG() << "Closing scene without saving: " << e.sceneIndex;
		_editorContext.CloseScene(e.sceneIndex);
	}
	else // Cancel
	{
		LOG_DEBUG() << "Close scene cancelled: " << e.sceneIndex;
	}
	
	return true;
}

bool EditorApplicationLayer::OnDestroyScene(const OnDestroySceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	_undoManager.EraseContext(e.registry);
	return false;
}

PopupResult EditorApplicationLayer::InvokePopupRequestSaveChanges(const int sceneIndex)
{
	// Get scene name for the message
	auto scene = _sceneManager->GetScene(sceneIndex);
	std::string sceneName = scene ? scene->GetFileName() : "Untitled";
	if (sceneName.empty())
		sceneName = "Untitled";
	
	std::string message = "Save changes to scene \"" + sceneName + "\"?\n\n";

	PopupResult result;

	// Create popup with custom buttons
	auto popup = std::make_shared<PopupWindow>("Save pending changes?", message);
	popup->AddButton("Yes", [&result]() { result = PopupResult::Yes; }, true);
	popup->AddButton("No", [&result]() { result = PopupResult::No; }, true);
	popup->AddButton("Cancel", [&result]() { result = PopupResult::Cancel; }, true);

	// Show blocking popup
	_popupManager.ShowPopupBlocking(popup);

	// Return the result as PopupResult enum
	return result;
}

bool EditorApplicationLayer::OnChangeActiveScene(const OnChangeActiveSceneEvent& e)
{
	LOG_TRACE() << e.GetName() << " received in editor layer.";
	_propertiesBar.SetContext(*_sceneManager->GetActiveScene());
	_sceneHierarchyPanel.SetContext(*_sceneManager->GetActiveScene());
	_editorContext.OnActiveSceneChanged();
	_cameraController.SetContext(_sceneManager->GetActiveScene() ? &_sceneManager->GetActiveScene()->GetActiveCamera() : nullptr);
	return false;
}

bool EditorApplicationLayer::OnRequestUndo(const RequestUndoEvent& e)
{
	_editorContext.Undo();
	return true;
}

bool EditorApplicationLayer::OnRequestRedo(const RequestRedoEvent& e)
{
	_editorContext.Redo();
	return true;
}
