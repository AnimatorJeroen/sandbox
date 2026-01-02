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
	_name = name;
}

std::string& Scene::GetName()
{
	return _name;
}

entt::entity Scene::CreateEntity()
{
	entt::entity e = _registry.create();
	// random float between 0 and 1
	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	_registry.emplace<DummyComponent>(e, DummyComponent{dist(rng)});
	return e;
}

