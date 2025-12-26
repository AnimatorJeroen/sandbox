#include "UIApplicationLayer.h"
#include <imgui/imgui.h>

UIApplicationLayer::UIApplicationLayer()
{
}


void UIApplicationLayer::OnUpdate(const float deltaTime)
{
}

void UIApplicationLayer::OnRender()
{
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Begin("Sandbox Controls");
    ImGui::Text("Adjust clear color:");
    ImGui::ColorEdit3("Background Color", (float*)&clear_color);
    ImGui::End();
}
