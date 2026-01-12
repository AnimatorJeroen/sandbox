#include "Scene.h"
#include <random>

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{
	for (const auto& shape : _shapes) {
		shape->Draw(recorder);
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
	
	// Add name component
	_registry.emplace<NameComponent>(e, name);
	
	// Add dummy component with random value
	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	_registry.emplace<DummyComponent>(e, DummyComponent{dist(rng)});
	
	return e;
}

