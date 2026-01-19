#pragma once

// Forward declare
class SceneManager;
class EditorContext;

namespace Core {
    class EventBus;
}

class MainMenu
{
public:
    MainMenu(Core::EventBus& eventBus, EditorContext& editorContext);
    void Render();
private:
    Core::EventBus& _eventBus;
    EditorContext& _editorContext;
};