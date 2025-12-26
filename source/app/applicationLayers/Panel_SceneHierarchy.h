#pragma once
#include "scene/Scene.h"

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& context);
	~Panel_SceneHierarchy() = default;
	void OnRender();
private:
	Scene& _scene;
};