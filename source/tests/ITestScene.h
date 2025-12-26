#pragma once
#include "scene/Scene.h"

class ITestScene
{
public:
	virtual void Setup() = 0;
	virtual void Teardown() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual ~ITestScene() = default;
};