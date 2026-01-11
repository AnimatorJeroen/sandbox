#pragma once
#include <imgui/imgui.h>
#include "core/event/EventBus.h"
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include <app/sceneLayer/SceneManager.h>

class OpenDocumentsTopBar
{
private:
    Core::EventBus& _eventBus;
    SceneManager& _sceneManager;
public:
    OpenDocumentsTopBar(SceneManager& sceneManager, Core::EventBus& eventBus) : _eventBus(eventBus), _sceneManager(sceneManager) {}

    void Render()
    {
        // Get all open scenes
        const auto& scenes = _sceneManager.GetAllScenes();
        size_t activeIndex = _sceneManager.GetActiveSceneIndex();

        if (scenes.empty())
            return;

        // Create a window for the tab bar with specific flags to make it a top bar
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight())); // Position below main menu bar
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetFrameHeight() + 5)); // Full width

        ImGui::Begin("##OpenDocuments", nullptr, windowFlags);

        // Create a tab bar with reorderable and closeable flags
        ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_Reorderable |
            ImGuiTabBarFlags_AutoSelectNewTabs |
            ImGuiTabBarFlags_FittingPolicyScroll;

        if (ImGui::BeginTabBar("OpenDocumentsTabBar", tabBarFlags))
        {
            // Track if we need to close a scene
            size_t sceneToClose = -1;
            size_t currentlySelectedTab = -1;

            // Iterate through all scenes and create tabs
            for (size_t i = 0; i < scenes.size(); ++i)
            {
                auto scene = scenes[i];
                if (!scene)
                    continue;

                // Get scene name - String64 has conversion operator to std::string
                auto sceneName = scene->GetName();
                if (sceneName.empty())
                    sceneName = "Untitled";

                // Create unique ID for each tab
                std::string tabId = sceneName.to_string() + "##" + std::to_string(i);

                // Don't use SetSelected flag - let the tab bar manage selection naturally
                ImGuiTabItemFlags tabFlags = 0;

                bool tabOpen = true;
                if (ImGui::BeginTabItem(tabId.c_str(),
                    scenes.size() > 1 ? &tabOpen : nullptr,
                    tabFlags))
                {
                    // This tab is currently selected
                    currentlySelectedTab = i;

                    ImGui::EndTabItem();
                }

                // Check if tab was closed
                if (!tabOpen && scenes.size() > 1)
                {
                    sceneToClose = i;
                }
            }

            // Only update active scene if the selected tab changed
            if (currentlySelectedTab != -1 && 
                currentlySelectedTab != activeIndex)
            {
                _sceneManager.SetActiveScene(currentlySelectedTab);
            }

            // Close scene after iteration to avoid invalidating iterator
            if (sceneToClose >= 0)
            {
                _eventBus.PushEvent<RequestCloseSceneEvent>(RequestCloseSceneEvent(sceneToClose));
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};