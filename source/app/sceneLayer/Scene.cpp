#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <core/Logger.h>

// C++17 compatible helper structs for Scene serialization
// Must be defined outside function scope for member templates to work
namespace {
	struct SceneSaveHelper {
		template<typename... Cs>
		static void save(std::ofstream& file, entt::registry& registry, std::tuple<Cs...>*) {
			auto archive = Core::ArchiveHelpers::MakeFullSnapshot<Cs...>(registry);
			
			// Write entity count
			uint64_t entityCount = archive.entities.size();
			file.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));

			// Write all entity IDs
			file.write(reinterpret_cast<const char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Write each component storage
			writeStorages(file, archive, std::index_sequence_for<Cs...>{});
		}
		
		template<typename... Cs, std::size_t... Is>
		static void writeStorages(std::ofstream& file, 
			const Core::SelectionArchive<Cs...>& archive,
			std::index_sequence<Is...>) {
			// Use dummy array initialization trick for C++17 fold expression alternative
			int dummy[] = {0, (writeStorageAtIndex<Is>(file, archive), 0)...};
			(void)dummy; // Suppress unused variable warning
		}
		
		template<std::size_t I, typename... Cs>
		static void writeStorageAtIndex(std::ofstream& file, const Core::SelectionArchive<Cs...>& archive) {
			const auto& storage = std::get<I>(archive.storages);
			uint64_t count = storage.size();
			file.write(reinterpret_cast<const char*>(&count), sizeof(count));

			for (const auto& [entity, component] : storage) {
				file.write(reinterpret_cast<const char*>(&entity), sizeof(entity));
				file.write(reinterpret_cast<const char*>(&component), sizeof(component));
			}
		}
	};

	struct SceneLoadHelper {
		template<typename... Cs>
		static void load(std::ifstream& file, entt::registry& registry, std::tuple<Cs...>*) {
			Core::SelectionArchive<Cs...> archive;

			// Read entity count
			uint64_t entityCount = 0;
			file.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));

			// Read all entity IDs
			archive.entities.resize(entityCount);
			file.read(reinterpret_cast<char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Read each component storage
			readStorages(file, archive, std::index_sequence_for<Cs...>{});

			// Restore all entities (including the scene entity)
			Core::ArchiveHelpers::RestoreEntitiesAndComponents(registry, archive);
		}
		
		template<typename... Cs, std::size_t... Is>
		static void readStorages(std::ifstream& file, 
			Core::SelectionArchive<Cs...>& archive,
			std::index_sequence<Is...>) {
			// Use dummy array initialization trick for C++17 fold expression alternative
			int dummy[] = {0, (readStorageAtIndex<Is>(file, archive), 0)...};
			(void)dummy; // Suppress unused variable warning
		}
		
		template<std::size_t I, typename... Cs>
		static void readStorageAtIndex(std::ifstream& file, Core::SelectionArchive<Cs...>& archive) {
			auto& storage = std::get<I>(archive.storages);
			uint64_t count = 0;
			file.read(reinterpret_cast<char*>(&count), sizeof(count));

			using ComponentType = typename std::tuple_element<I, std::tuple<Cs...>>::type;
			storage.resize(count);
			for (uint64_t i = 0; i < count; ++i) {
				entt::entity entity;
				ComponentType component;
				file.read(reinterpret_cast<char*>(&entity), sizeof(entity));
				file.read(reinterpret_cast<char*>(&component), sizeof(component));
				storage[i] = {entity, component};
			}
		}
	};
}

void Scene::UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight)
{
	auto& camera = GetActiveCamera();
	m_ViewMatrix = glm::lookAt(camera.position, camera.target, camera.up);
	float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	m_ProjectionMatrix = glm::perspective(glm::radians(camera.fov), aspect, camera.nearPlane, camera.farPlane);
}

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{

	int i = 0;
	for (auto entity : _registry.view<Transform>()) {
		auto& transform = _registry.get<Transform>(entity);

		float x = transform.Position.x;
		float y = transform.Position.y;


		// Scale down coordinates to fit in camera view
		// Camera is at (0,5,10) looking at (0,0,0)
		// So we need coordinates near the origin
		float worldX = (x - 200.0f) * 0.01f;  // Center around 0, scale down
		float worldY = (y - 200.0f) * 0.01f;  // Center around 0, scale down

		recorder.PolygonBegin(Core::Vec3{ worldX - 0.1f, worldY - 0.1f, -5.0f }, 2.0f,
			{ 0.0f, 1.0f, 0.0f, 1.0f }, true
		);
		recorder.PolygonPoint(Core::Vec3{ worldX + 0.1f, worldY - 0.1f, -5.0f });
		recorder.PolygonPoint(Core::Vec3{ worldX + 0.1f, worldY + 0.1f, -5.0f });
		recorder.PolygonPoint(Core::Vec3{ worldX - 0.1f, worldY + 0.1f, -5.0f });
		recorder.PolygonEnd(Core::Vec3{ worldX - 0.1f, worldY - 0.1f, -5.0f });

		// Draw 2D line (explicitly using Vec2)
		recorder.Line(
			Core::Vec3{ worldX, worldY, -5.0f },
			Core::Vec3{ worldX + 0.1f, worldY, -5.0f },
			2,
			{ 1.0f, 0.0f, 0.0f, 1.0f }
		);

		// Draw 3D cube for each entity
		recorder.Cube(transform.GetTransform(), { 0.2f + i * 0.1f, 0.5f, 1.0f, 1.0f });

		i++;
	}
}

void Scene::SetName(const std::string& name)
{
	data()._name = name;
}

const String64& Scene::GetName()
{
	return data()._name;
}

const std::string& Scene::GetFileName()
{
	return _fileName;
}

entt::entity Scene::CreateEntity()
{
	static int _entityCounter = 0;
	// Generate unique name
	std::string entityName = "Entity_" + std::to_string(_entityCounter++);
	return CreateEntity(entityName);
}

entt::entity Scene::CreateEntity(const std::string& name)
{
	entt::entity e = _registry.create();

	_registry.emplace<Core::UUID>(e);
	
	// Add name component
	_registry.emplace<NameComponent>(e, name);

	// Generate random position between 0 and 400
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 0.5f);
	
	float x = dis(gen);
	float y = dis(gen);

	_registry.emplace<Transform>(e, Transform{ vec3{x, y, 0.0f} });

	return e;
}

entt::entity Scene::CreateCameraEntity()
{
	entt::entity e = _registry.create();

	_registry.emplace<Core::UUID>(e);
	_registry.emplace<NameComponent>(e, "Camera");
	_registry.emplace<CameraComponent>(e);

	return e;
}

inline CameraComponent& Scene::GetActiveCamera() { return _registry.get<CameraComponent>(_activeCamera); }

bool Scene::SaveToFile(const std::string& filepath) const
{
	try {
		// Create directory if it doesn't exist
		std::filesystem::path filePath(filepath);
		std::filesystem::create_directories(filePath.parent_path());

		std::ofstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR() << "Failed to open file for writing: " << filepath;
			return false;
		}

		// Write file format version
		uint32_t version = 1;
		file.write(reinterpret_cast<const char*>(&version), sizeof(version));

		// Create archive with all component types (including SceneData on _sceneEntity)
		// No need to sync camera - SceneData is already the source of truth
		SceneSaveHelper::save(file, const_cast<entt::registry&>(_registry), static_cast<AppComponentTypes*>(nullptr));

		bool success = file.good();
		file.close();

		if (success) {
			LOG_DEBUG() << "Scene saved to: " << filepath;
		} else {
			LOG_ERROR() << "Error writing scene to file: " << filepath;
		}

		return success;
	}
	catch (const std::exception& e) {
		LOG_ERROR() << "Exception saving scene: " << e.what();
		return false;
	}
}

bool Scene::LoadFromFile(const std::string& filepath)
{
	try {
		std::ifstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR() << "Failed to open file for reading: " << filepath;
			return false;
		}

		// Read and validate file format version
		uint32_t version = 0;
		file.read(reinterpret_cast<char*>(&version), sizeof(version));
		if (version != 1) {
			LOG_ERROR() << "Unsupported scene file version: " << version;
			return false;
		 }

		// Clear existing scene
		_registry.clear();

		// Load entities and components using RestoreEntitiesAndComponents
		SceneLoadHelper::load(file, _registry, static_cast<AppComponentTypes*>(nullptr));

		// Find the scene entity by looking for SceneData component
		auto sceneDataView = _registry.view<SceneData>();
		if (sceneDataView.empty()) {
			LOG_ERROR() << "No SceneData component found in loaded scene";
			return false;
		}

		// Set _sceneEntity to the entity with SceneData
		_sceneEntity = sceneDataView.front();
		
		// Find and set the first camera entity as active camera
		auto cameraView = _registry.view<CameraComponent>();
		if (!cameraView.empty()) {
			_activeCamera = cameraView.front();
		}
		else {
			// No camera entity found, create a default one
			_activeCamera = CreateCameraEntity();
		}

		 // Camera settings are already in SceneData - just update matrices
		UpdateCameraMatrices(800, 600);

		bool success = file.good();
		file.close();

		if (success) {
			LOG_DEBUG() << "Scene loaded from: " << filepath;
		} else {
			LOG_ERROR() << "Error reading scene from file: " << filepath;
		}

		return success;
	}
	catch (const std::exception& e) {
		LOG_ERROR() << "Exception loading scene: " << e.what();
		return false;
	}
}

