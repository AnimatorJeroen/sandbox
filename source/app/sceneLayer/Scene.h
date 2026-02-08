#pragma once
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Registry.h"

#include <entt/entt.hpp>
#include <core/renderer/DrawCommandRecorder.h>
#include <core/UUID.h>
#include <core/applicator/SelectionArchive.h>
#include "components/Components.h"
#include "Entity.h"
#include "FbxPlayer.h"

class Scene
{
	public:
		Scene() {
			_sceneEntity = _registry.Create(); 
			_registry.emplace<SceneData>(_sceneEntity);
			
			 // Create default camera entity
			_activeCamera = CreateCameraEntity();
		}
		~Scene() {
			_registry.clear();
		}

		Scene(const Scene&) {}
		
		void Draw(Core::DrawCommandRecorder& recorder);
		
		// Camera/View management
		void UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight);
		void UpdateMatrices();
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		
		// Active camera management
		CameraComponent& GetActiveCamera();
		void SetActiveCamera(Entity camera) { _activeCamera = camera.GetHandle(); }

		 // FBX Animation playback
		void UpdateFbxPlayer(float deltaTime) { _fbxPlayer.Update(*this, deltaTime); }
		FbxPlayer& GetFbxPlayer() { return _fbxPlayer; }

		// Scene serialization using SelectionArchive
		bool SaveToFile(const std::string& filepath) const;
		bool LoadFromFile(const std::string& filepath);

		// Access entt registry
		inline Core::Registry& GetRegistry() { return _registry; }
		inline const Core::Registry& GetRegistry() const { return _registry; }

        // Create a new entity with DummyComponent and unique name
        Entity CreateEntity(const bool addTransform = true);
        
        // Create a named entity
        Entity CreateEntity(const String64& name, const bool addTransform = true);

        // Create a camera entity with CameraComponent
        Entity CreateCameraEntity();

		std::set<Entity> GetAllEntities() const;
		
		// Get an entity by its UUID
		Entity GetEntity(uint64_t uuid) const;

		template<typename... Components>
		auto GetEntitiesOfType()
		{
			auto view = _registry.view<Components...>();
			std::set<Entity> entities;
			for (auto e : view)
			{
				// Exclude the scene entity itself
				if (_sceneEntity != e)
					entities.insert(Entity(e, const_cast<Core::Registry*>(&_registry)));
			}
			return entities;
		}

		Entity GetSceneEntity() const { return Entity(_sceneEntity, const_cast<Core::Registry*>(&_registry)); }

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

		// Hierarchy management
		void SetParent(Entity child, Entity parent);
		void RebuildChildrenForEntity(Entity entity);
		void RebuildChildrenForAllEntities();


	private:
		// Recursive helper for hierarchical transform updates
		void UpdateEntityHierarchyRecursive(entt::entity entity, const glm::mat4& parentWorldMatrix);
		void RebuildSkeletonForEntity(Entity skeletonEntity);
		void RebuildAnimChannelsForEntity(Entity animEntity);

		entt::entity _sceneEntity;
		entt::entity _activeCamera = entt::null;
		int a = 0;
		Core::Registry _registry{};
		std::string _filepath; // File path for this scene (empty for unsaved scenes)
		std::string _fileName;
		
		 // FBX Animation player
		FbxPlayer _fbxPlayer;
		
		// 3D Camera matrices (computed from CameraComponent)
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;

		SceneData& data() { return _registry.get<SceneData>(_sceneEntity); }
		const SceneData& data() const { return _registry.get<SceneData>(_sceneEntity); }
};