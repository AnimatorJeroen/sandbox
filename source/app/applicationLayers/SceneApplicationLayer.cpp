#include "SceneApplicationLayer.h"

SceneApplicationLayer::SceneApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), testScene(*ctx.Get<Scene>().get())
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
