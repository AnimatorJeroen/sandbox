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
	for (auto entity : _registry.view<NameComponent>()) {
		// Just a demo: draw a circle at position based on UUID
		auto uuid = _registry.get<Core::UUID>(entity);

		float x = static_cast<float>((20)); // Assuming window width 800
		float y = static_cast<float>((100) + i * 5); // Assuming window height 600
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

	return e;
}

