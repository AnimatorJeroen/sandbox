#pragma once
#include "app/sceneLayer/SceneManager.h"
#include <memory>

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(const SceneManager& sceneManager);
	~Panel_SceneHierarchy() = default;
	void Render();
private:
	const SceneManager& _sceneManager;
};