#include "pch.h"
#include "EditorApplicationLayer.h"
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
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
	_sceneHierarchyPanel.Render();
	
	RenderGizmos();

	// Render popups last so they appear on top of everything
	_popupManager.Render();
}

void EditorApplicationLayer::RenderGizmos()
{
	auto scene = _sceneManager->GetActiveScene();
	if (!scene)
		return;
	auto& registry = scene->GetRegistry();
	auto selectedEntities = _editorContext.GetSelectedEntities();
	
	if (selectedEntities.empty())
		return;
	
	ImGuizmo::BeginFrame();
	// Setup ImGuizmo
	int windowWidth = 1280, windowHeight = 720;
	//_editorContext.Ge
	ImGuizmo::SetRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
	
	// Get camera matrices  
	auto& cameraComponent = scene->GetActiveCamera();
	const glm::mat4 viewMatrix = glm::lookAt(cameraComponent.position, cameraComponent.target, cameraComponent.up);
	const glm::mat4 projectionMatrix = glm::perspective(glm::radians(cameraComponent.fov), 
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 
		cameraComponent.nearPlane, cameraComponent.farPlane);
	
	// For each selected entity, render gizmo
	for (const auto& entity : selectedEntities)
	{
		if (registry.any_of<Transform>(entity))
		{
			auto& transform = registry.get<Transform>(entity);
			glm::mat4 modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::translate(modelMatrix, transform.Position);
			modelMatrix = glm::scale(modelMatrix, transform.Scale);

			// Use translate operation
			ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
			
			// Manipulate
			ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
				operation, ImGuizmo::LOCAL, glm::value_ptr(modelMatrix));
			

			static bool imGuizmoActivate = false;
			// If manipulated, decompose and update transform
			if (ImGuizmo::IsUsing())
			{
				if (!imGuizmoActivate)
				{
					_editorContext.BeginUndo();
					imGuizmoActivate = true;

					if (operation == ImGuizmo::TRANSLATE)
					{
						_applicator.SetField(entity, "Transform.Position", transform.Position);
					}
					if (operation == ImGuizmo::SCALE)
					{
						_applicator.SetField(entity, "Transform.Scale", transform.Scale);
					}
				}
				glm::vec3 translation, rotation, scale;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix),
					glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

				transform.Position = translation;
				transform.Scale = scale;
			}
			else if(imGuizmoActivate)
			{
				if (operation == ImGuizmo::TRANSLATE)
				{
					_applicator.SetField(entity, "Transform.Position", transform.Position);
				}
				if (operation == ImGuizmo::SCALE)
				{
					_applicator.SetField(entity, "Transform.Scale", transform.Scale);
				}
				_editorContext.EndUndo();
				imGuizmoActivate = false;
			}
		}
	}
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
