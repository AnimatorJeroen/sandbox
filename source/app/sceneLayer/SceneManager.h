#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include <string>
#include "app/event/SceneEvent.h"

//forward declare
namespace Core
{
    class EventBus;
}

class SceneManager
{
public:
    explicit SceneManager(Core::EventBus& eventBus);

    std::shared_ptr<Scene> GetActiveScene() const;

    std::shared_ptr<Scene> GetScene(int index) const;

    const std::vector<std::shared_ptr<Scene>>& GetAllScenes() const;

    int GetActiveSceneIndex() const;

    std::shared_ptr<Scene> CreateNewScene(const std::string& name, bool makeActive);
    
    bool SaveScene(int index, const std::string& filepath);

    bool LoadScene(const char* filepath, bool makeActive = true);

    bool SaveActiveScene(const std::string& filepath);

    void SetActiveScene(int index);

    void CloseScene(int index);

    int GetSceneCount() const;

private:
    std::vector<std::shared_ptr<Scene>> _scenes;
    int _activeSceneIndex = -1;
    Core::EventBus& _eventBus;
    
    void NotifySceneChanged();
};
