#pragma once
#include "Scene.h"
#include "core/event/EventBus.h"
#include <memory>
#include <vector>
#include <string>

class SceneManager
{
public:
    explicit SceneManager(Core::EventBus& eventBus);
    
    // Get the currently active scene (never returns null, always has at least one scene)
    std::shared_ptr<Scene> GetActiveScene() const { 
        return _scenes[_activeSceneIndex]; 
    }
    
    // Get scene by index
    std::shared_ptr<Scene> GetScene(size_t index) const {
        if (index >= _scenes.size()) return nullptr;
        return _scenes[index];
    }
    
    // Get all open scenes
    const std::vector<std::shared_ptr<Scene>>& GetAllScenes() const {
        return _scenes;
    }
    
    // Get active scene index
    size_t GetActiveSceneIndex() const {
        return _activeSceneIndex;
    }
    
    // Create a new scene and optionally make it active
    std::shared_ptr<Scene> CreateNewScene(const std::string& name = "Untitled", bool makeActive = true) {
        auto scene = std::make_shared<Scene>();
        scene->SetName(name);
        _scenes.push_back(scene);
        
        if (makeActive) {
            SetActiveScene(_scenes.size() - 1);
        }
        
        return scene;
    }
    
    // Load a scene from file and add to open scenes
    bool LoadScene(const std::string& filepath, bool makeActive = true);
    
    // Save the active scene
    bool SaveActiveScene(const std::string& filepath);
    
    // Save a specific scene
    bool SaveScene(size_t index, const std::string& filepath);
    
    // Set the active scene by index
    void SetActiveScene(size_t index) {
        if (index >= _scenes.size()) return;
        
        size_t oldIndex = _activeSceneIndex;
        _activeSceneIndex = index;
        
        // Push event to event bus if scene changed
        if (oldIndex != index) {
            NotifySceneChanged();
        }
    }
    
    // Close a scene by index
    void CloseScene(size_t index) {
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
    
    // Get number of open scenes
    size_t GetSceneCount() const {
        return _scenes.size();
    }

private:
    std::vector<std::shared_ptr<Scene>> _scenes{ std::make_shared<Scene>() }; // Always have at least one scene
    size_t _activeSceneIndex = 0;
    Core::EventBus& _eventBus;
    
    void NotifySceneChanged();
};
