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

class Scene
{
	public:
		Scene() = default;
		
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

        // Editable direct scene color channel (for demo) 
        float sceneColor{1.0f};

	private:
		std::string _name;
		std::vector<std::shared_ptr<IShape>> _shapes;
		int a = 0;
		entt::registry _registry{};
};