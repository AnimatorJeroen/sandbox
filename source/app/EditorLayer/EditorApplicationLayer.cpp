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
//ui panels
_mainMenu(*ctx.Get<Core::EventBus>().get(), ctx.Get<Core::Window>()->GetNativeWindowHandle(), ctx.Get<SceneManager>().get()),
_sceneHierarchyPanel(*_sceneManager->GetActiveScene(), _applicator),
_openDocumentsTopBar(*_sceneManager, *ctx.Get<Core::EventBus>().get())
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
	REGISTER_CALLBACK(_eventBus, RequestUndoEvent, OnRequestUndo);
	REGISTER_CALLBACK(_eventBus, RequestRedoEvent, OnRequestRedo);
}

void EditorApplicationLayer::OnUpdate(const float deltaTime)
{
}

void EditorApplicationLayer::OnRender()
{
	_mainMenu.Render();
	_openDocumentsTopBar.Render();
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
			SaveSceneAs(_sceneManager.get(), _eventBus, _windowHandle);
		}
		else if (e.mods & Core::KMOD_CONTROL)
		{
			SaveScene(_sceneManager.get(), _eventBus, _windowHandle);
		}
	}
	else if (e.key == 'O' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			OpenScene(_sceneManager.get(), _eventBus, _windowHandle);
		}
	}
	else if (e.key == 'C' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			auto sel = _sceneHierarchyPanel.GetSelectedEntities();
			if (sel.empty())
				return true;
			_applicator.CopyToClipboard(sel);
		}
	}
	else if (e.key == 'X' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			auto sel = _sceneHierarchyPanel.GetSelectedEntities();
			if(sel.empty())
				return true;

			_applicator.CopyToClipboard(sel);

			_applicator.BeginUndo();
			_applicator.CaptureDelete(sel);
			_applicator.EndUndo();
		}
	}
	else if (e.key == 'V' && !e.repeated) {
		if (e.mods & Core::KMOD_CONTROL)
		{
			_applicator.BeginUndo();
			_applicator.PasteFromClipboard(Core::ClipboardType::Entities);
			_applicator.EndUndo();
		}
	}

	return true;
}

void EditorApplicationLayer::SaveScene(SceneManager* sceneManager, Core::EventBus& eventBus, void* windowHandle)
{
	auto scene = sceneManager->GetActiveScene();
	if (scene && !scene->GetFilepath().empty())
	{
		// Save to current filepath
		eventBus.PushEvent<RequestSaveSceneEvent>(
			RequestSaveSceneEvent(scene->GetFilepath()));
	}
	else
	{
		SaveSceneAs(sceneManager, eventBus, windowHandle);
	}
}

void EditorApplicationLayer::SaveSceneAs(SceneManager* sceneManager, Core::EventBus& eventBus, void* windowHandle)
{
	auto scene = sceneManager->GetActiveScene();
	if (scene == nullptr)
		return;

	Core::BrowserWindow browserWindow(windowHandle);
	auto result = browserWindow.SaveFile(
		"Save Scene As",
		{
			Core::FileFilter("Scene Files", "*.scene"),
			Core::FileFilter("Binary Files", "*.dat"),
			Core::FileFilter("All Files", "*.*")
		},
		"",
		"scene");

	if (result.has_value())
	{
		eventBus.PushEvent<RequestSaveSceneEvent>(
			RequestSaveSceneEvent(result.value()));
	}
}

void EditorApplicationLayer::OpenScene(SceneManager* sceneManager, Core::EventBus& eventBus, void* windowHandle)
{
	Core::BrowserWindow browserWindow(windowHandle);
	auto result = browserWindow.OpenFile(
		"Open Scene",
		{
			Core::FileFilter("Scene Files", "*.scene"),
			Core::FileFilter("Binary Files", "*.dat"),
			Core::FileFilter("All Files", "*.*")
		});

	if (result.has_value())
	{
		eventBus.PushEvent<RequestLoadSceneEvent>(
			RequestLoadSceneEvent(result.value()));
	}
}

void EditorApplicationLayer::RevertScene(SceneManager* sceneManager, Core::EventBus& eventBus, void* windowHandle)
{
	auto scene = sceneManager->GetActiveScene();
	if (scene == nullptr)
		return;

	auto filePath = scene->GetFilepath();
	if (filePath.empty())
		return;

	eventBus.PushImmediateEvent<RequestCloseSceneEvent>(
		RequestCloseSceneEvent(sceneManager->GetActiveSceneIndex()));

	eventBus.PushImmediateEvent<RequestLoadSceneEvent>(
		RequestLoadSceneEvent(filePath));
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
	_undoManager.SetContext(_sceneManager->GetActiveScene()->GetRegistry());
	_undoManager.Clear();
	return false;
}

bool EditorApplicationLayer::OnRequestUndo(const RequestUndoEvent& e)
{
	bool handled = _undoManager.Undo();
	LOG_DEBUG() << e.GetName() << " Undo handled: " << handled;
	return true;
}

bool EditorApplicationLayer::OnRequestRedo(const RequestRedoEvent& e)
{
	bool handled = _undoManager.Redo();
	LOG_DEBUG() << e.GetName() << " Redo handled: " << handled;
	return true;
}
