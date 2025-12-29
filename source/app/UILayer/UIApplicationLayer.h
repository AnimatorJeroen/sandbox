#pragma once
#include "core/IApplicationLayer.h"
#include "panels/Panel_SceneHierarchy.h"
#include "MainMenu.h"
#include <core/event/EventBus.h>
#include "core/event/MouseEvent.h"
#include "core/event/KeyEvent.h"
#include "app/event/EditorEvent.h"

class SceneManager;

class UIApplicationLayer : public Core::IApplicationLayer
{
public:
	explicit UIApplicationLayer(Core::LayerContext& ctx);
	~UIApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

	bool OnMouseDownEvent(const Core::MouseDownEvent& e);
	bool OnMouseUpEvent(const Core::MouseUpEvent& e);
	bool OnMouseMoveEvent(const Core::MouseMoveEvent& e);
	bool OnMouseScrollEvent(const Core::MouseScrollEvent& e);
	bool OnKeyDownEvent(const Core::KeyDownEvent& e);
	bool OnKeyUpEvent(const Core::KeyUpEvent& e);
	bool OnKeyCharacterEvent(const Core::KeyCharacterEvent& e);

	bool OnEditorRequestSaveSceneEvent(const EditorRequestSaveSceneEvent& e);
	bool OnEditorRequestLoadSceneEvent(const EditorRequestLoadSceneEvent& e);
	bool OnEditorSceneReloadedEvent(const EditorSceneReloadedEvent& e);

private:
	std::shared_ptr<SceneManager> _sceneManager;
	Panel_SceneHierarchy _sceneHierarchyPanel;
	MainMenu _mainMenu;
	Core::EventBus& _eventBus;
};