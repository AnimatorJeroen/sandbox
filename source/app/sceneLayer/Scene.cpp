#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{
	//for (const auto& shape : _shapes) {
	//	shape->Draw(recorder);
	//}

	int i = 0;
	for (auto entity : _registry.view<Transform>()) {
		auto& transform = _registry.get<Transform>(entity);

		float x = transform.Position.x; // Assuming window width 800
		float y = transform.Position.y + 100 + i * 15; // Assuming window height 600

		recorder.PolygonBegin({ x - 10.0f, y - 10.0f },2.0f,
			{ 0.0f, 1.0f, 0.0f, 1.0f }, true
		);
		recorder.PolygonPoint({ x + 10.0f, y - 10.0f });
		recorder.PolygonPoint({ x + 10.0f, y + 10.0f });
		recorder.PolygonPoint({ x - 10.0f, y + 10.0f });
		recorder.PolygonEnd({ x - 10.0f, y - 10.0f });

		recorder.Line(
			{ x - 10.0f, y },
			{ x + 10.0f, y },
			2,
			{ 1.0f, 0.0f, 0.0f, 1.0f }
		);

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

