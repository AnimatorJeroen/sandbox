#include "UIApplicationLayer.h"
#include <imgui/imgui.h>

UIApplicationLayer::UIApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), m_sceneHierarchyPanel(*ctx.Get<Scene>().get())
{
}

void UIApplicationLayer::OnUpdate(const float deltaTime)
{
}

void UIApplicationLayer::OnRender()
{
	m_sceneHierarchyPanel.OnRender();
}
