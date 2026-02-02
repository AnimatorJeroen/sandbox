#pragma once
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt/entt.hpp>
#include <core/renderer/DrawCommandRecorder.h>
#include <core/UUID.h>
#include <core/applicator/SelectionArchive.h>
#include "components/Components.h"
#include "Entity.h"

class Scene
{
	public:
		Scene() {
			_sceneEntity = _registry.create(); 
			_registry.emplace<Core::UUID>(_sceneEntity);
			_registry.emplace<SceneData>(_sceneEntity);
			
			 // Create default camera entity
			_activeCamera = CreateCameraEntity();
			
			// Initialize default camera matrices
			UpdateCameraMatrices(800, 600); // Default viewport size
		}
		~Scene() {
			_registry.clear();
		}

		Scene(const Scene&) {}
		
		void Draw(Core::DrawCommandRecorder& recorder);
		
		// Camera/View management
		void UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight);
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		
		// Active camera management
		CameraComponent& GetActiveCamera();
		void SetActiveCamera(Entity camera) { _activeCamera = camera.GetHandle(); }

		// Scene serialization using SelectionArchive
		bool SaveToFile(const std::string& filepath) const;
		bool LoadFromFile(const std::string& filepath);

		// Access entt registry
		inline entt::registry& GetRegistry() { return _registry; }
		inline const entt::registry& GetRegistry() const { return _registry; }

        // Create a new entity with DummyComponent and unique name
        Entity CreateEntity();
        
        // Create a named entity
        Entity CreateEntity(const std::string& name);
        
        // Create a camera entity with CameraComponent
        Entity CreateCameraEntity();

		std::set<Entity> GetAllEntities() const;

		Entity GetSceneEntity() const { return Entity(_sceneEntity, const_cast<entt::registry*>(&_registry)); }

		void SetName(const std::string& name);
		const String64& GetName();
		const std::string& GetFileName();
		inline const float GetSceneColor() { return data().sceneColor; }
		inline void SetSceneColor(float color) { data().sceneColor = color; }

		// Filepath management
		void SetFilepath(const std::string& filepath) {
			_filepath = filepath; _fileName = std::filesystem::path(filepath).filename().string();
		}
		const std::string& GetFilepath() const { return _filepath; }

	private:
		entt::entity _sceneEntity;
		entt::entity _activeCamera = entt::null;
		int a = 0;
		entt::registry _registry{};
		std::string _filepath; // File path for this scene (empty for unsaved scenes)
		std::string _fileName;
		
		// 3D Camera matrices (computed from CameraComponent)
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;

		SceneData& data() { return _registry.get<SceneData>(_sceneEntity); }
		const SceneData& data() const { return _registry.get<SceneData>(_sceneEntity); }
};