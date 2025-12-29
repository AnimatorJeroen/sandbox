#pragma once
#include <imgui/imgui.h>
#include "core/event/EventBus.h"
#include "core/event/ApplicationEvent.h"
#include "app/event/EditorEvent.h"


class MainMenu
{
private:
    Core::EventBus& _eventBus;
public:
    MainMenu(Core::EventBus& eventBus) : _eventBus(eventBus) {}

    void Render()
    {
        if(ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Save"))
                {
					_eventBus.PushEvent<EditorRequestSaveSceneEvent>(EditorRequestSaveSceneEvent("saved files/scene.dat"));
                }
                if(ImGui::MenuItem("Load"))
                {
					_eventBus.PushEvent<EditorRequestLoadSceneEvent>(EditorRequestLoadSceneEvent("saved files/scene.dat"));
                }
                if (ImGui::MenuItem("Exit"))
                {
                    _eventBus.PushEvent<Core::ApplicationCloseEvent>(Core::ApplicationCloseEvent());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Option A");
                ImGui::MenuItem("Option B");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};