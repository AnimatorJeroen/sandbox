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

// Dummy component to track with the registry
struct DummyComponent {
    float value{0.0f};
    // Editable color channels
    float r{1.0f}, g{1.0f}, b{1.0f}, a{1.0f};
};

struct SceneData {
	float sceneColor = 0.f;
	float testValue = 42.f;
};

class Scene
{
	public:
		entt::entity sceneEntity;

		Scene() {
			sceneEntity = _registry.create(); 
			_registry.emplace<SceneData>(sceneEntity, SceneData{});
		}
		~Scene() = default;
		Scene(const Scene&) {}
		
		void Draw(Core::DrawCommandRecorder& recorder);
		void SetName(const std::string& name);
		std::string& GetName();

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(_name, _shapes);
		}

		inline std::vector<std::shared_ptr<IShape>>& GetShapes() { return _shapes; }

		// Access entt registry
		inline entt::registry& GetRegistry() { return _registry; }
		inline const entt::registry& GetRegistry() const { return _registry; }

        // Create a new entity and attach a DummyComponent with random value
        entt::entity CreateEntity();

        // Returns reference to the scene data for access
        const SceneData& GetSceneData() const { return _registry.get<SceneData>(sceneEntity); } // Updated return type to reference

		const entt::entity& GetSceneEntity() const { return sceneEntity; }
	private:
		std::string _name;
		std::vector<std::shared_ptr<IShape>> _shapes;
		int a = 0;
		entt::registry _registry{};
};