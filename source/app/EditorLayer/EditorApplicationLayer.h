#pragma once
#include "core/IApplicationLayer.h"
#include "panels/Panel_SceneHierarchy.h"
#include "MainMenu.h"
#include "EditorContext.h"
#include "PopupManager.h"
#include "CameraController.h"
#include "core/event/MouseEvent.h"
#include "core/event/KeyEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include "core/applicator/Applicator.h"
#include "core/applicator/UndoManager.h"

#include "app/sceneLayer/types/Types.h"
#include "OpenDocumentsTopBar.h"

#include <ImGuizmo/ImGuizmo.h>
#include "PropertiesBar.h"

class SceneManager;
class EventBus;

// Forward declare enum
enum class PopupResult : int;

// Layout information for editor panels
struct EditorLayout
{
	float frameHeight = 0.0f;
	float mainMenuHeight = 0.0f;
	float openDocumentsHeight = 0.0f;
	float propertiesBarHeight = 0.0f;
	float hierarchyStartY = 0.0f;
	float viewportWidth = 0.0f;
	float viewportHeight = 0.0f;
};

class EditorApplicationLayer : public Core::IApplicationLayer
{
public:
	explicit EditorApplicationLayer(Core::LayerContext& ctx);
	~EditorApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

	bool OnMouseDownEvent(const Core::MouseDownEvent& e);
	bool OnMouseUpEvent(const Core::MouseUpEvent& e);
	bool OnMouseMoveEvent(const Core::MouseMoveEvent& e);
	bool OnMouseScrollEvent(const Core::MouseScrollEvent& e);
	bool OnKeyDownEvent(const Core::KeyDownEvent& e);
	
	bool OnKeyUpEvent(const Core::KeyUpEvent& e);
	bool OnKeyCharacterEvent(const Core::KeyCharacterEvent& e);

	bool OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e);
	bool OnRequestCloseSceneEvent(const RequestCloseSceneEvent& e);
	bool OnDestroyScene(const OnDestroySceneEvent& e);
	PopupResult InvokePopupRequestSaveChanges(const int sceneIndex);
	bool OnChangeActiveScene(const OnChangeActiveSceneEvent& e);
	bool OnRequestApplicationCloseEvent(const Core::RequestApplicationCloseEvent& e);
	bool OnApplicationCloseEvent(const Core::ApplicationCloseEvent& e);

	bool OnRequestUndo(const RequestUndoEvent& e);
	bool OnRequestRedo(const RequestRedoEvent& e);

	// Access to popup manager for other components
	PopupManager& GetPopupManager() { return _popupManager; }

private:
	void RenderImGuizmo();
	EditorLayout CalculateLayout() const;
	
	// Core services
	std::shared_ptr<SceneManager> _sceneManager;
	Core::EventBus& _eventBus;
	Core::Applicator<AppFieldTypes, AppComponentTypes> _applicator;
	Core::UndoManager<AppFieldTypes> _undoManager;
	void* _windowHandle;

	// Editor context - shared state and operations
	EditorContext _editorContext;

	// UI panels
	Panel_SceneHierarchy _sceneHierarchyPanel;
	MainMenu _mainMenu;
	OpenDocumentsTopBar _openDocumentsTopBar;
	PropertiesBar _propertiesBar;
	
	// Popup system
	PopupManager _popupManager;
	
	// Camera controller
	CameraController _cameraController;
	
	// Mouse button state tracking
	bool _isLeftMouseDown = false;
	bool _isMiddleMouseDown = false;
	bool _isRightMouseDown = false;
	double _lastMouseX = 0.0f;
	double _lastMouseY = 0.0f;
};