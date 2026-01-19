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

    std::shared_ptr<Scene> GetScene(size_t index) const;

    const std::vector<std::shared_ptr<Scene>>& GetAllScenes() const;

    size_t GetActiveSceneIndex() const;

    std::shared_ptr<Scene> CreateNewScene(const std::string& name, bool makeActive);
    
    bool SaveScene(size_t index, const std::string& filepath);

    bool LoadScene(const char* filepath, bool makeActive = true);

    bool SaveActiveScene(const std::string& filepath);

    void SetActiveScene(size_t index);

    void CloseScene(size_t index);

    size_t GetSceneCount() const;

private:
    std::vector<std::shared_ptr<Scene>> _scenes;
    size_t _activeSceneIndex = -1;
    Core::EventBus& _eventBus;
    
    void NotifySceneChanged();
};
