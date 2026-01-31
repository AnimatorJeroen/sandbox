#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"
#include <glm/gtc/matrix_transform.hpp>

void Scene::UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight)
{
	// Update view matrix
	m_ViewMatrix = glm::lookAt(m_CameraPosition, m_CameraTarget, m_CameraUp);
	
	// Update projection matrix
	float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	m_ProjectionMatrix = glm::perspective(glm::radians(m_CameraFOV), aspect, m_CameraNear, m_CameraFar);
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

