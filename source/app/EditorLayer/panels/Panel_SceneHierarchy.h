#pragma once
#include "app/sceneLayer/Scene.h"
#include <core/undo/Applicator.h>
#include <memory>
#include <set>

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene, Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
private:
	Scene* _scene;
	Core::Applicator<AppFieldTypes, AppComponentTypes>& _applicator;
	std::set<entt::entity> _selectedEntities;
	entt::entity _lastClickedEntity = entt::null;
};