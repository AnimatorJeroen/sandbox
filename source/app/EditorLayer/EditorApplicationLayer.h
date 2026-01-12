#pragma once
#include "core/IApplicationLayer.h"
#include "panels/Panel_SceneHierarchy.h"
#include "MainMenu.h"
#include <core/event/EventBus.h>
#include "core/event/MouseEvent.h"
#include "core/event/KeyEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include "core/undo/Applicator.h"
#include "core/undo/UndoManager.h"

#include "app/sceneLayer/types/Types.hpp"
#include "OpenDocumentsTopBar.h"

class SceneManager;

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

	bool OnRequestSaveSceneEvent(const RequestSaveSceneEvent& e);
	bool OnRequestLoadSceneEvent(const RequestLoadSceneEvent& e);
	bool OnChangeActiveScene(const OnChangeActiveSceneEvent& e);

	bool OnRequestUndo(const RequestUndoEvent& e);
	bool OnRequestRedo(const RequestRedoEvent& e);

private:
	std::shared_ptr<SceneManager> _sceneManager;
	Panel_SceneHierarchy _sceneHierarchyPanel;
	MainMenu _mainMenu;
	OpenDocumentsTopBar _openDocumentsTopBar;

	Core::EventBus& _eventBus;
	Core::Applicator<AppValueTypes> _applicator;
	Core::UndoManager<AppValueTypes> _undoManager;
};