#include "pch.h"
#include "PropertiesBar.h"
#include "app/sceneLayer/Scene.h"
#include "EditorContext.h"

void PropertiesBar::Render()
{
    // Create a window for the properties bar with specific flags to make it a bar below document tabs
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    float topBarHeight = ImGui::GetFrameHeight() + 5;
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight() + topBarHeight)); // Position below document tabs
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetFrameHeight() + 5)); // Full width

    ImGui::Begin("##GizmoProperties", nullptr, windowFlags);

    // Add padding for better appearance
    ImGui::AlignTextToFramePadding();
    
    // World/Local mode toggle button
    const auto& imGuizmoMode = _editorContext.GetImGuizmoMode();
    const char* modeLabel = (imGuizmoMode == ImGuizmo::WORLD) ? "World" : "Local";
    if (ImGui::Button(modeLabel, ImVec2(80, 0)))
    {
        _editorContext.SetImGuizmoMode((imGuizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD);
    }
    
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Toggle between World and Local gizmo mode");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    
    // Operation mode indicators (optional visual feedback)
    ImGui::Text("|");
    ImGui::SameLine();

    const auto& imGuizmoOperation = _editorContext.GetImGuizmoOperation();
    const char* operationLabel =
        (imGuizmoOperation == ImGuizmo::TRANSLATE) ? "Translate (W)" :
        (imGuizmoOperation == ImGuizmo::ROTATE) ? "Rotate (E)" :
        (imGuizmoOperation == ImGuizmo::SCALE) ? "Scale (R)" : "Unknown";
    
    ImGui::Text("%s", operationLabel);

    ImGui::End();
}

void PropertiesBar::SetContext(Scene& scene)
{
    _scene = &scene;
}
