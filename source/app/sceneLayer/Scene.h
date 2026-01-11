#pragma once
#include <vector>
#include <memory>
#include <string>
#include <random>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include "shape/IShape.h"
#include <entt/entt.hpp>
#include <core/renderer/DrawCommandRecorder.h>
#include "types/Types.hpp"

// Dummy component to track with the registry
struct DummyComponent {
    float value{0.0f};
    // Editable color channels
    float r{1.0f}, g{1.0f}, b{1.0f}, a{1.0f};
};

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
			_registry.emplace<SceneData>(_sceneEntity, SceneData{});
		}
		~Scene() = default;
		Scene(const Scene&) {}
		
		void Draw(Core::DrawCommandRecorder& recorder);

		template<class Archive>
		void serialize(Archive& archive)
		{
			// 2. Serialize scene-level components 
			auto& sceneData = _registry.get<SceneData>(_sceneEntity);
			archive(sceneData, _shapes);
		}

		inline std::vector<std::shared_ptr<IShape>>& GetShapes() { return _shapes; }

		// Access entt registry
		inline entt::registry& GetRegistry() { return _registry; }
		inline const entt::registry& GetRegistry() const { return _registry; }

        // Create a new entity and attach a DummyComponent with random value
        entt::entity CreateEntity();

		const entt::entity& GetSceneEntity() const { return _sceneEntity; }

		void SetName(const std::string& name);
		const String64& GetName();
		inline const float GetSceneColor() { return data().sceneColor; }
		inline void SetSceneColor(float color) { data().sceneColor = color; }

		// Filepath management
		void SetFilepath(const std::string& filepath) { _filepath = filepath; }
		const std::string& GetFilepath() const { return _filepath; }

	private:
		entt::entity _sceneEntity;
		std::vector<std::shared_ptr<IShape>> _shapes;
		int a = 0;
		entt::registry _registry{};
		std::string _filepath; // File path for this scene (empty for unsaved scenes)

		SceneData& data() { return _registry.get<SceneData>(_sceneEntity); } // Updated return type to reference
};