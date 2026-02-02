#include "pch.h"
#include "Panel_SceneHierarchy.h"
#include "app/editorLayer/EditorContext.h"
#include <core/applicator/Applicator.h>
#include "app/sceneLayer/Scene.h"
#include <imgui/imgui.h>
#include <core/UUID.h>
#include <algorithm>

Panel_SceneHierarchy::Panel_SceneHierarchy(Scene& scene, EditorContext& editorContext) 
    : _scene(&scene), _editorContext(editorContext)
{
}

void Panel_SceneHierarchy::SetContext(Scene& scene)
{
    _scene = &scene;
}

void Panel_SceneHierarchy::Render()
{
    static char inputTextbuffer[64];
    static Scene* lastScene = nullptr;
    static float sceneColor = _scene->GetSceneColor();
    if (!_scene)
        return;

    ImGui::Begin("Scene Hierarchy");
	
    sceneColor = _scene->GetSceneColor();
    ImGui::InputFloat("Scene Color: %f", &sceneColor);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(_scene->GetSceneEntity().GetHandle(), "Scene.sceneColor", sceneColor);
        _editorContext.EndUndo();
    }
	// Update scene name when input changes
    strncpy_s(inputTextbuffer, _scene->GetName().data, sizeof(inputTextbuffer) - 1);
    ImGui::InputText("input field", inputTextbuffer, sizeof(inputTextbuffer));
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(_scene->GetSceneEntity().GetHandle(), "Scene.name", String64(inputTextbuffer));
        _editorContext.EndUndo();
    }

    if (ImGui::Button("Add Entity"))
    {
        _editorContext.BeginUndo();
        auto e = _scene->CreateEntity();
        _editorContext.applicator().CaptureCreate({e.GetHandle()});
        _editorContext.EndUndo();
    }

    ImGui::Separator();
    ImGui::Text("Scene Entities:");
    
    // Get all entities from scene
    auto entitiesSet = _scene->GetAllEntities();
    
    // Convert to vector and sort by UUID
    std::vector<Entity> entities(entitiesSet.begin(), entitiesSet.end());
    std::sort(entities.begin(), entities.end(), [](const Entity& a, const Entity& b) {
        return a.UUID().value < b.UUID().value;
    });

    // Get selection from EditorContext
    auto& selectedEntities = _editorContext.GetSelectedEntities();
    
	bool deleteEntitiesPressed = false;
    for (size_t i = 0; i < entities.size(); ++i) {
        Entity entity = entities[i];
        const NameComponent* nameComp = entity.HasComponent<NameComponent>() 
            ? &entity.GetComponent<NameComponent>() 
            : nullptr;
        
        // Skip the scene entity
        if (entity == _scene->GetSceneEntity())
            continue;
        // Skip the camera entity
        if (entity.HasComponent<CameraComponent>())
            continue;

        // Display entity with its name
        ImGui::PushID(static_cast<int>(entity.UUID().value));
        
        // Determine tree node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow 
                                 | ImGuiTreeNodeFlags_SpanAvailWidth
                                 | ImGuiTreeNodeFlags_Leaf;
        
        // Add selected flag if this entity is selected
        if (selectedEntities.find(entity) != selectedEntities.end()) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        bool nodeOpen = ImGui::TreeNodeEx(
            nameComp ? nameComp->name.data : "Unnamed",
            flags
        );
        
        // Handle selection
        if (ImGui::IsItemClicked()) {
            bool isShiftDown = ImGui::GetIO().KeyShift;
            bool isCtrlDown = ImGui::GetIO().KeyCtrl;
            
            if (isShiftDown && _lastClickedEntity) {
                // Shift+Click: Select range from last clicked to current
                // Find indices of last clicked and current entity
                auto lastIt = std::find(entities.begin(), entities.end(), _lastClickedEntity);
                
                if (lastIt != entities.end()) {
                    // Determine range direction
                    size_t startIdx = std::min(static_cast<size_t>(lastIt - entities.begin()), i);
                    size_t endIdx = std::max(static_cast<size_t>(lastIt - entities.begin()), i);
                    
                    // Build new selection
                    std::set<Entity> newSelection;
                    
                    // Keep existing selection if holding ctrl
                    if (isCtrlDown) {
                        newSelection = selectedEntities;
                    }
                    
                    // Select all entities in range
                    for (size_t idx = startIdx; idx <= endIdx; ++idx) {
                        Entity e = entities[idx];
                        if (e != _scene->GetSceneEntity()) {
                            newSelection.insert(e);
                        }
                    }
                    
                    _editorContext.SetSelection(newSelection);
                }
            }
            else if (isCtrlDown) {
                // Ctrl+Click: Toggle selection
                std::set<Entity> newSelection = selectedEntities;
                if (selectedEntities.find(entity) != selectedEntities.end()) {
                    newSelection.erase(entity);
                } else {
                    newSelection.insert(entity);
                }
                _editorContext.SetSelection(newSelection);
                _lastClickedEntity = entity;
            }
            else {
                // Normal click: Replace selection
                std::set<Entity> newSelection;
                newSelection.insert(entity);
                _editorContext.SetSelection(newSelection);
                _lastClickedEntity = entity;
            }
        }
        
        // Handle delete key - check once per frame, not per entity
        if (!ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            deleteEntitiesPressed = true;
        }

        // Show entity ID in tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Entity ID: %u", static_cast<uint32_t>(entt::to_integral(entity.GetHandle())));

            // Show UUID
            ImGui::Text("UUID: %llu", entity.UUID().value);

            // Show selection info
            if (selectedEntities.size() > 1) {
                ImGui::Text("(%zu entities selected)", selectedEntities.size());
            }
            ImGui::EndTooltip();
        }
        
        // If node was opened (for future children support)
        if (nodeOpen) {
            // Future: render child entities here
            ImGui::TreePop();
        }
        
        ImGui::PopID();
    }

    if(deleteEntitiesPressed && !selectedEntities.empty())
    {
        _editorContext.DeleteSelection();
        
        // Clear last clicked if it was deleted
        if (selectedEntities.find(_lastClickedEntity) == selectedEntities.end()) {
            _lastClickedEntity = Entity::Null();
        }
	}

    ImGui::End();
}