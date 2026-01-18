#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"

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

