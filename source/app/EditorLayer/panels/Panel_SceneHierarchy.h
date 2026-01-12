#pragma once
#include "app/sceneLayer/Scene.h"
#include <core/undo/Applicator.h>
#include <memory>

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene, Core::Applicator<AppValueTypes>& applicator);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
private:
	Scene* _scene;
	Core::Applicator<AppValueTypes>& _applicator;
};