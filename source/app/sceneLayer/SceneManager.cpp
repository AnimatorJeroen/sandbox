#include "SceneManager.h"
#include "core/event/EventBus.h"
#include "core/serializer/Serializer.h"
#include "app/event/SceneEvent.h"
#include <iostream>

SceneManager::SceneManager(Core::EventBus& eventBus) : _eventBus(eventBus)
{
}

std::shared_ptr<Scene> SceneManager::GetActiveScene() const
{
    return _scenes[_activeSceneIndex];
}

std::shared_ptr<Scene> SceneManager::GetScene(size_t index) const
{
    if (index >= _scenes.size()) return nullptr;
    return _scenes[index];
}

const std::vector<std::shared_ptr<Scene>>& SceneManager::GetAllScenes() const
{
    return _scenes;
}

size_t SceneManager::GetActiveSceneIndex() const
{
    return _activeSceneIndex;
}

std::shared_ptr<Scene> SceneManager::CreateNewScene(const std::string& name, bool makeActive)
{
    auto scene = std::make_shared<Scene>();
    scene->SetName(name);
    _scenes.push_back(scene);
    
    if (makeActive) {
        SetActiveScene(_scenes.size() - 1);
    }
    
    return scene;
}

void SceneManager::SetActiveScene(size_t index)
{
    if (index >= _scenes.size()) return;
    
    size_t oldIndex = _activeSceneIndex;
    _activeSceneIndex = index;
    
    // Push event to event bus if scene changed
    if (oldIndex != index) {
        NotifySceneChanged();
    }
}

void SceneManager::CloseScene(size_t index)
{
    if (_scenes.size() <= 1) {
        // Always keep at least one scene open
        return;
    }
    
    if (index >= _scenes.size()) return;
    
    _scenes.erase(_scenes.begin() + index);
    
    // Adjust active index if needed
    if (_activeSceneIndex >= _scenes.size()) {
        _activeSceneIndex = _scenes.size() - 1;
        NotifySceneChanged();
    } else if (index <= _activeSceneIndex && _activeSceneIndex > 0) {
        _activeSceneIndex--;
    }
}

size_t SceneManager::GetSceneCount() const
{
    return _scenes.size();
}

void SceneManager::NotifySceneChanged()
{
    _eventBus.PushEvent<OnChangeActiveSceneEvent>(OnChangeActiveSceneEvent());
}

bool SceneManager::LoadScene(const std::string& filepath, bool makeActive)
{
    try {
        auto scene = Core::Serializer::Deserialize<Scene>(filepath);
        
        if (!scene) {
            std::cerr << "Failed to deserialize scene from: " << filepath << std::endl;
            return false;
        }
        
        _scenes.push_back(scene);
        
        if (makeActive) {
            SetActiveScene(_scenes.size() - 1);
        }
        
        std::cout << "Scene loaded from: " << filepath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return false;
    }
}

bool SceneManager::SaveActiveScene(const std::string& filepath)
{
    return SaveScene(_activeSceneIndex, filepath);
}

bool SceneManager::SaveScene(size_t index, const std::string& filepath)
{
    if (index >= _scenes.size()) {
        std::cerr << "Invalid scene index: " << index << std::endl;
        return false;
    }
    
    try {
        bool success = Core::Serializer::Serialize<Scene>(_scenes[index], filepath);
        
        if (success) {
            std::cout << "Scene saved to: " << filepath << std::endl;
        } else {
            std::cerr << "Failed to serialize scene to: " << filepath << std::endl;
        }
        
        return success;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving scene: " << e.what() << std::endl;
        return false;
    }
}
