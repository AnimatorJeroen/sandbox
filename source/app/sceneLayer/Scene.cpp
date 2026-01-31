#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <core/Logger.h>

void Scene::UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight)
{
	// Get camera settings from SceneData
	const auto& sceneData = data();
	
	// Update view matrix
	m_ViewMatrix = glm::lookAt(sceneData.cameraPosition, sceneData.cameraTarget, m_CameraUp);
	
	// Update projection matrix
	float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	m_ProjectionMatrix = glm::perspective(glm::radians(sceneData.cameraFOV), aspect, m_CameraNear, m_CameraFar);
}

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{

	int i = 0;
	for (auto entity : _registry.view<Transform>()) {
		auto& transform = _registry.get<Transform>(entity);

		float x = transform.Position.x;
		float y = transform.Position.y + 0.100 + i * 0.15;

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
		glm::mat4 cubeTransform = glm::mat4(1.0f);
		cubeTransform = glm::translate(cubeTransform, glm::vec3(worldX, worldY, -7.0f));
		cubeTransform = glm::rotate(cubeTransform, glm::radians(i * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		cubeTransform = glm::scale(cubeTransform, glm::vec3(0.5f));
		recorder.Cube(cubeTransform, { 0.2f + i * 0.1f, 0.5f, 1.0f, 1.0f });

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
	static std::uniform_real_distribution<float> dis(0.0f, 400.0f);
	
	float x = dis(gen);
	float y = dis(gen);

	_registry.emplace<Transform>(e, Transform{ vec4{x, y, 0.0f, 1.0f}});

	return e;
}

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
		auto saveEntities = [&]<typename... Cs>(std::tuple<Cs...>*) {
			auto archive = Core::ArchiveHelpers::MakeFullSnapshot<Cs...>(
				const_cast<entt::registry&>(_registry));
			
			// Write entity count
			uint64_t entityCount = archive.entities.size();
			file.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));

			// Write all entity IDs
			file.write(reinterpret_cast<const char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Write each component storage
			auto writeStorages = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
				([&] {
					const auto& storage = std::get<Is>(archive.storages);
					uint64_t count = storage.size();
					file.write(reinterpret_cast<const char*>(&count), sizeof(count));

					for (const auto& [entity, component] : storage) {
						file.write(reinterpret_cast<const char*>(&entity), sizeof(entity));
						file.write(reinterpret_cast<const char*>(&component), sizeof(component));
					}
				}(), ...);
			};
			writeStorages(std::index_sequence_for<Cs...>{});
		};
		saveEntities(static_cast<AppComponentTypes*>(nullptr));

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
		auto loadEntities = [&]<typename... Cs>(std::tuple<Cs...>*) {
			Core::SelectionArchive<Cs...> archive;

			// Read entity count
			uint64_t entityCount = 0;
			file.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));

			// Read all entity IDs
			archive.entities.resize(entityCount);
			file.read(reinterpret_cast<char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Read each component storage
			auto readStorages = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
				([&] {
					auto& storage = std::get<Is>(archive.storages);
					uint64_t count = 0;
					file.read(reinterpret_cast<char*>(&count), sizeof(count));

					storage.resize(count);
					for (uint64_t i = 0; i < count; ++i) {
						entt::entity entity;
						typename std::tuple_element_t<Is, std::tuple<Cs...>> component;
						file.read(reinterpret_cast<char*>(&entity), sizeof(entity));
						file.read(reinterpret_cast<char*>(&component), sizeof(component));
						storage[i] = {entity, component};
					}
				}(), ...);
			};
			readStorages(std::index_sequence_for<Cs...>{});

			// Restore all entities (including the scene entity)
			Core::ArchiveHelpers::RestoreEntitiesAndComponents(_registry, archive);
		};
		loadEntities(static_cast<AppComponentTypes*>(nullptr));

		// Find the scene entity by looking for SceneData component
		auto sceneDataView = _registry.view<SceneData>();
		if (sceneDataView.empty()) {
			LOG_ERROR() << "No SceneData component found in loaded scene";
			return false;
		}

		// Set _sceneEntity to the entity with SceneData
		_sceneEntity = sceneDataView.front();

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

