#pragma once
#include <vector>
#include <memory>
#include <string>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <entt/entt.hpp>
#include <core/renderer/DrawCommandRecorder.h>
#include <core/UUID.h>
#include "components/Components.h"

struct SceneData {

	float sceneColor = 0.f;
	String64 _name;
	template<class Archive> 
	void serialize(Archive& ar) {
		ar(_name, sceneColor);
	}
};

class Scene
{
	public:
		Scene() {
			_sceneEntity = _registry.create(); 
			_registry.emplace<Core::UUID>(_sceneEntity);
			_registry.emplace<SceneData>(_sceneEntity, SceneData{});
			
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
		void SetCameraPosition(const glm::vec3& position) { m_CameraPosition = position; }
		void SetCameraTarget(const glm::vec3& target) { m_CameraTarget = target; }
		const glm::vec3& GetCameraPosition() const { return m_CameraPosition; }
		const glm::vec3& GetCameraTarget() const { return m_CameraTarget; }

		template<class Archive>
		void save(Archive& archive) const
		{
			//Serialize scene-level components 
			auto& sceneData = _registry.get<SceneData>(_sceneEntity);
			
			// Collect entity data for saving
			std::vector<Core::UUID> uuids;
			std::vector<NameComponent> names;

			for (auto entity : _registry.view<NameComponent>()) {
				if (entity != _sceneEntity) {

					uuids.push_back(_registry.get<Core::UUID>(entity));

					if (_registry.all_of<NameComponent>(entity)) {
						names.push_back(_registry.get<NameComponent>(entity));
					} else {
						names.push_back(NameComponent{}); // Generate new if missing
					}
				}
			}

			// Serialize the data
			archive(sceneData, uuids, names);
		}

		template<class Archive>
		void load(Archive& archive)
		{
			auto& sceneData = _registry.get<SceneData>(_sceneEntity);
			std::vector<Core::UUID> uuids;
			std::vector<NameComponent> names;

			archive(sceneData, uuids, names);

			// Recreate entities from loaded data
			for (size_t i = 0; i < uuids.size(); ++i) {
				entt::entity newEntity = _registry.create();
				_registry.emplace<Core::UUID>(newEntity, uuids[i]);

				if (i < names.size()) {
					_registry.emplace<NameComponent>(newEntity, names[i]);
				} else {
					_registry.emplace<NameComponent>(newEntity); // Generate new if missing
				}
			}
		}

		// Access entt registry
		inline entt::registry& GetRegistry() { return _registry; }
		inline const entt::registry& GetRegistry() const { return _registry; }

        // Create a new entity with DummyComponent and unique name
        entt::entity CreateEntity();
        
        // Create a named entity
        entt::entity CreateEntity(const std::string& name);

		const entt::entity& GetSceneEntity() const { return _sceneEntity; }

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
		int a = 0;
		entt::registry _registry{};
		std::string _filepath; // File path for this scene (empty for unsaved scenes)
		std::string _fileName;
		
		// 3D Camera matrices
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		glm::vec3 m_CameraPosition{0.0f, 5.0f, 10.0f};
		glm::vec3 m_CameraTarget{0.0f, 0.0f, 0.0f};
		glm::vec3 m_CameraUp{0.0f, 1.0f, 0.0f};
		float m_CameraFOV = 45.0f;
		float m_CameraNear = 0.1f;
		float m_CameraFar = 100.0f;

		SceneData& data() { return _registry.get<SceneData>(_sceneEntity); } // Updated return type to reference
};