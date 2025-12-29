#include "Scene.h"

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

