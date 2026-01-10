#pragma once
#include <imgui/imgui.h>
#include "core/event/EventBus.h"
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include "core/BrowserWindow.h"


class MainMenu
{
private:
    Core::EventBus& _eventBus;
    Core::BrowserWindow _browserWindow;
public:
    MainMenu(Core::EventBus& eventBus, void* windowHandle) 
        : _eventBus(eventBus), _browserWindow(windowHandle) {}

    void Render()
    {
        if(ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Save"))
                {
					_eventBus.PushEvent<RequestSaveSceneEvent>(RequestSaveSceneEvent("saved files/scene.dat"));
                }
                if(ImGui::MenuItem("Save Scene As..."))
                {
                    auto result = _browserWindow.SaveFile(
                        "Save Scene As",
                        {
                            Core::FileFilter("Scene Files", "*.scene"),
                            Core::FileFilter("Binary Files", "*.dat"),
                            Core::FileFilter("All Files", "*.*")
                        },
                        "",
                        "scene");
                    
                    if (result.has_value())
                    {
                        _eventBus.PushEvent<RequestSaveSceneEvent>(
                            RequestSaveSceneEvent(result.value()));
                    }
                }
                if(ImGui::MenuItem("Open Scene..."))
                {
                    auto result = _browserWindow.OpenFile(
                        "Open Scene",
                        {
                            Core::FileFilter("Scene Files", "*.scene"),
                            Core::FileFilter("Binary Files", "*.dat"),
                            Core::FileFilter("All Files", "*.*")
                        });
                    
                    if (result.has_value())
                    {
                        _eventBus.PushEvent<RequestLoadSceneEvent>(
                            RequestLoadSceneEvent(result.value()));
                    }
                }
                if (ImGui::MenuItem("Exit"))
                {
                    _eventBus.PushEvent<Core::ApplicationCloseEvent>(Core::ApplicationCloseEvent());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo"))
                {
                    _eventBus.PushEvent<RequestUndoEvent>(RequestUndoEvent());
                }
                if (ImGui::MenuItem("Redo"))
                {
                    _eventBus.PushEvent<RequestRedoEvent>(RequestRedoEvent());
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};