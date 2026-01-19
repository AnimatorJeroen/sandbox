#pragma once
#include <imgui/imgui.h>
#include "core/event/EventBus.h"
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include <app/sceneLayer/SceneManager.h>

// Forward declaration
class EditorContext;

class OpenDocumentsTopBar
{
private:
    Core::EventBus& _eventBus;
    SceneManager& _sceneManager;
    EditorContext& _editorContext;
    size_t _lastActiveIndex = static_cast<size_t>(-1); // Track last known active scene
    
    void ShowCloseSceneConfirmation(size_t sceneIndex);
    
public:
    OpenDocumentsTopBar(SceneManager& sceneManager, Core::EventBus& eventBus, EditorContext& editorContext) 
        : _eventBus(eventBus), _sceneManager(sceneManager), _editorContext(editorContext) {}

    void Render();
};