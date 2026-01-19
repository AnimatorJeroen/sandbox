#include "pch.h"
#include "EditorApplicationLayer.h"
#include <imgui/imgui.h>
#include <core/serializer/Serializer.h>
#include "app/sceneLayer/SceneManager.h"
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
//popup system
_popupManager()
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
}

void EditorApplicationLayer::OnUpdate(const float deltaTime)
{
}

void EditorApplicationLayer::OnRender()
{
	_mainMenu.Render();
	_openDocumentsTopBar.Render();
	_sceneHierarchyPanel.Render();
	
	// Render popups last so they appear on top of everything
	_popupManager.Render();
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
	else if (e.key == 'O' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_editorContext.OpenScene();
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
		size_t sceneIndex = _sceneManager->GetSceneCount() - 1;
		PopupResult result = InvokePopupRequestSaveChanges(sceneIndex);

		if (result == PopupResult::Yes)
		{
			LOG_DEBUG() << "Saving and closing scene...";
			_editorContext.SaveScene(sceneIndex);
			_sceneManager->CloseScene(sceneIndex);
		}
		else if (result == PopupResult::No)
		{
			LOG_DEBUG() << "Closing scene without saving...";
			_sceneManager->CloseScene(sceneIndex);
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

	// Show blocking confirmation
	PopupResult result = InvokePopupRequestSaveChanges(e.sceneIndex);
	
	if (result == PopupResult::Yes)
	{
		// TODO: Save the scene before closing
		LOG_DEBUG() << "Saving and closing scene: " << e.sceneIndex;
		_sceneManager->CloseScene(e.sceneIndex);
	}
	else if (result == PopupResult::No)
	{
		LOG_DEBUG() << "Closing scene without saving: " << e.sceneIndex;
		_sceneManager->CloseScene(e.sceneIndex);
	}
	else // Cancel
	{
		LOG_DEBUG() << "Close scene cancelled: " << e.sceneIndex;
	}
	
	return true;
}

PopupResult EditorApplicationLayer::InvokePopupRequestSaveChanges(const size_t sceneIndex)
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
	_sceneHierarchyPanel.SetContext(*_sceneManager->GetActiveScene());
	_editorContext.OnActiveSceneChanged();
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
