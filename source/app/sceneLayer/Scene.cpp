#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"
#include <glm/gtc/matrix_transform.hpp>

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{
	//for (const auto& shape : _shapes) {
	//	shape->Draw(recorder);
	//}

	int i = 0;
	for (auto entity : _registry.view<Transform>()) {
		auto& transform = _registry.get<Transform>(entity);

		float x = transform.Position.x;
		float y = transform.Position.y + 100 + i * 15;

		// Draw 2D filled square
		recorder.PolygonBegin({ x - 10.0f, y - 10.0f }, 2.0f,
			{ 0.0f, 1.0f, 0.0f, 1.0f }, true
		);
		recorder.PolygonPoint({ x + 10.0f, y - 10.0f });
		recorder.PolygonPoint({ x + 10.0f, y + 10.0f });
		recorder.PolygonPoint({ x - 10.0f, y + 10.0f });
		recorder.PolygonEnd({ x - 10.0f, y - 10.0f });

		// Draw 2D line
		recorder.Line(
			{ x - 10.0f, y },
			{ x + 10.0f, y },
			2,
			{ 1.0f, 0.0f, 0.0f, 1.0f }
		);

		// Draw 3D cube for each entity
		glm::mat4 cubeTransform = glm::mat4(1.0f);
		cubeTransform = glm::translate(cubeTransform, glm::vec3((x) * 0.01f, (y) * -0.01f, -5.0f));
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

	_registry.emplace<Transform>(e, Transform{x, y});

	return e;
}

