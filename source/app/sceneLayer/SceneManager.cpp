#include "pch.h"
#include "SceneManager.h"
#include "core/event/EventBus.h"
#include "core/serializer/Serializer.h"

#include <core/Logger.h>

SceneManager::SceneManager(Core::EventBus& eventBus) : _eventBus(eventBus) {}

std::shared_ptr<Scene> SceneManager::GetActiveScene() const
{
    if (_activeSceneIndex >= _scenes.size()) return nullptr;
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
    if (index >= _scenes.size()) return;
    
    _scenes.erase(_scenes.begin() + index);
    
    // Adjust active index if needed
    if (_activeSceneIndex >= _scenes.size()) {
        _activeSceneIndex = _scenes.size() - 1;
    } else if (index <= _activeSceneIndex && _activeSceneIndex > 0) {
        _activeSceneIndex--;
    }
    // Notify scene changed - will be handled immediately before frame update
    NotifySceneChanged();
}

size_t SceneManager::GetSceneCount() const
{
    return _scenes.size();
}

void SceneManager::NotifySceneChanged()
{
    LOG_INFO() << "Activated scene with index [" << _activeSceneIndex << "]";
    // Use PushImmediateEvent so listeners are notified before OnUpdate/OnRender
    // This prevents stale scene references
    _eventBus.PushImmediateEvent<OnChangeActiveSceneEvent>(OnChangeActiveSceneEvent());
}

bool SceneManager::LoadScene(const char* filepath, bool makeActive)
{
    // Check if scene with this filepath is already open
    for (size_t i = 0; i < _scenes.size(); ++i) {
        if (_scenes[i]->GetFilepath() == filepath) {
            LOG_INFO() << "Scene already open, switching to it: " << filepath;
            if (makeActive) {
                SetActiveScene(i);
            }
            return true;
        }
    }
    
    try {
        auto scene = Core::Serializer::Deserialize<Scene>(filepath);
        
        if (!scene) {
            LOG_ERROR() << "Failed to deserialize scene from: " << filepath;
            return false;
        }
        
        // Set the filepath for the loaded scene
        scene->SetFilepath(filepath);
        
        _scenes.push_back(scene);
        
        if (makeActive) {
            SetActiveScene(_scenes.size() - 1);
        }
        
        LOG_INFO() << "Scene loaded from: " << filepath;
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR() << "Error loading scene: " << e.what();
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
        LOG_ERROR() << "Invalid scene index: " << index;
        return false;
    }
    
    try {
        bool success = Core::Serializer::Serialize<Scene>(_scenes[index], filepath);
        
        if (success) {
            // Update the scene's filepath after successful save
            _scenes[index]->SetFilepath(filepath);
            LOG_INFO() << "Scene saved to: " << filepath;
        } else {
            LOG_ERROR() << "Failed to serialize scene to: " << filepath;
        }
        
        return success;
    }
    catch (const std::exception& e) {
        LOG_ERROR() << "Error saving scene: " << e.what();
        return false;
    }
}
