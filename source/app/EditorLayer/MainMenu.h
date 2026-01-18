#pragma once

// Forward declare
class SceneManager;

namespace Core {
    class EventBus;
}


class MainMenu
{
public:
    MainMenu(Core::EventBus& eventBus, void* windowHandle, SceneManager* sceneManager = nullptr);
    void Render();
private:
    Core::EventBus& _eventBus;
    SceneManager* _sceneManager;
    void* _windowHandle;

};