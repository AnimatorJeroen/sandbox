#include "pch.h"
#include "MainMenu.h"
#include <imgui/imgui.h>
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include "app/sceneLayer/SceneManager.h"
#include "core/event/EventBus.h"
#include "EditorApplicationLayer.h"

MainMenu::MainMenu(Core::EventBus& eventBus, void* windowHandle, SceneManager* sceneManager)
    : _eventBus(eventBus), _windowHandle(windowHandle), _sceneManager(sceneManager)
{
}

void MainMenu::Render()
{
    if(ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {

            if(ImGui::MenuItem("Save", "Ctrl+S", false))
            {
                EditorApplicationLayer::SaveScene(_sceneManager, _eventBus, _windowHandle);
            }
            
            if(ImGui::MenuItem("Save Scene As..."))
            {
               EditorApplicationLayer::SaveSceneAs(_sceneManager, _eventBus, _windowHandle);
            }
            
            if(ImGui::MenuItem("Open Scene..."))
            {
                EditorApplicationLayer::OpenScene(_sceneManager, _eventBus, _windowHandle);
            }

            bool canRevert = _sceneManager && _sceneManager->GetActiveScene() &&
                !_sceneManager->GetActiveScene()->GetFilepath().empty();
            if(ImGui::MenuItem("Revert", nullptr, false, canRevert))
            {
                EditorApplicationLayer::RevertScene(_sceneManager, _eventBus, _windowHandle);
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit"))
            {
                _eventBus.PushEvent<Core::ApplicationCloseEvent>(Core::ApplicationCloseEvent());
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
                _eventBus.PushEvent<RequestUndoEvent>(RequestUndoEvent());
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z"))
            {
                _eventBus.PushEvent<RequestRedoEvent>(RequestRedoEvent());
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}
