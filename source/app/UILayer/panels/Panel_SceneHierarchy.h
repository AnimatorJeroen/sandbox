#pragma once
#include "app/sceneLayer/Scene.h"
#include <memory>

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
private:
	Scene* _scene;
};