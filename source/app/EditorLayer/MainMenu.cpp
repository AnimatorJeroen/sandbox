#include "pch.h"
#include "MainMenu.h"
#include "EditorContext.h"
#include "PopupManager.h"
#include <imgui/imgui.h>
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"
#include "app/event/UndoEvent.h"
#include "app/sceneLayer/SceneManager.h"
#include "core/event/EventBus.h"

MainMenu::MainMenu(Core::EventBus& eventBus, EditorContext& editorContext)
    : _eventBus(eventBus), _editorContext(editorContext)
{
}

void MainMenu::Render()
{
    if(ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {

            if(ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                _editorContext.NewScene("Untitled");
			}

			int activeSceneIndex = _editorContext.sceneManager().GetActiveSceneIndex();
            if(ImGui::MenuItem("Save", "Ctrl+S", false, activeSceneIndex != -1))
            {
                _editorContext.SaveScene(activeSceneIndex);
            }

            if(ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false, activeSceneIndex != -1))
            {
                _editorContext.SaveSceneAs(activeSceneIndex);
            }
            
            if(ImGui::MenuItem("Open Scene..."))
            {
                _editorContext.OpenScene();
            }

            bool canRevert = _editorContext.sceneManager().GetActiveScene() &&
                !_editorContext.sceneManager().GetActiveScene()->GetFilepath().empty();
            if(ImGui::MenuItem("Revert", nullptr, false, canRevert))
            {
                _editorContext.RevertScene();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit"))
            {
                _eventBus.PushEvent<Core::RequestApplicationCloseEvent>(Core::RequestApplicationCloseEvent());
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
            
            ImGui::Separator();
            
            bool hasSelection = !_editorContext.GetSelectedEntities().empty();
            
            if (ImGui::MenuItem("Cut", "Ctrl+X", false, hasSelection))
            {
                _editorContext.Cut();
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, hasSelection))
            {
                _editorContext.Copy();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V"))
            {
                _editorContext.Paste();
            }
            if (ImGui::MenuItem("Delete", "Del", false, hasSelection))
            {
                _editorContext.DeleteSelection();
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                auto* popupManager = _editorContext.GetPopupManager();
                if (popupManager)
                {
                    popupManager->ShowInfo(
                        "About",
                        "Scene Editor v1.0\n\nA powerful scene editing tool with undo/redo support."
                    );
                }
            }
            
            if (ImGui::MenuItem("Test Popup"))
            {
                auto* popupManager = _editorContext.GetPopupManager();
                if (popupManager)
                {
                    popupManager->ShowConfirmation(
                        "Test Confirmation",
                        "This is a test confirmation dialog.\nDo you want to proceed?",
                        []() {
                            // User clicked OK
                        },
                        []() {
                            // User clicked Cancel
                        }
                    );
                }
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}
