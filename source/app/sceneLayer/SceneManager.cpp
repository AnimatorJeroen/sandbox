#include "SceneManager.h"
#include "core/serializer/Serializer.h"
#include <iostream>

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
