#include "SceneApplicationLayer.h"
#include <tests/TestScene1.h>


static TestScene1 testScene;

SceneApplicationLayer::SceneApplicationLayer()
{
	testScene.Setup();
}

void SceneApplicationLayer::OnUpdate(const float deltaTime)
{
	testScene.Update(deltaTime);
}

void SceneApplicationLayer::OnRender()
{
	testScene.Render();
}
