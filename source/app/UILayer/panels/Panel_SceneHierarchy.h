#pragma once
#include "app/sceneLayer/Scene.h"

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& context);
	~Panel_SceneHierarchy() = default;
	void Render();
private:
	Scene& _scene;
};