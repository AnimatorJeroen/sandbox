#include "pch.h"
#include "Panel_SceneHierarchy.h"
#include "app/editorLayer/EditorContext.h"
#include <core/applicator/Applicator.h>
#include "app/sceneLayer/Scene.h"
#include <imgui/imgui.h>
#include <core/UUID.h>
#include <algorithm>

#include <app/sceneLayer/shape/Circle.h>

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
        _editorContext.applicator().SetField(_scene->GetSceneEntity(), "Scene.sceneColor", sceneColor);
        _editorContext.EndUndo();
    }
	// Update scene name when input changes
    strncpy_s(inputTextbuffer, _scene->GetName().data, sizeof(inputTextbuffer) - 1);
    ImGui::InputText("input field", inputTextbuffer, sizeof(inputTextbuffer));
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        //_scene->SetName(std::string(inputTextbuffer));
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(_scene->GetSceneEntity(), "Scene.name", String64(inputTextbuffer));
        _editorContext.EndUndo();
    }
	
    if (ImGui::Button("Add Circle"))
    {
        static int posIncr = 0;
        auto newCircle = std::make_shared<Circle>(
            glm::vec2{ 150.0f + posIncr, 150.0f },  // center
            15.0f,                        // radius
            true,                         // filled
            Core::ColorRGBA{ 1.0f, 0.0f, 0.0f, 1.0f },  // color
            32,                           // num_segments
            1.0f                          // thickness
        );
        _scene->GetShapes().push_back(newCircle);
        posIncr += 20;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Entity"))
    {
        _editorContext.BeginUndo();
        auto e = _scene->CreateEntity();
        _editorContext.applicator().CaptureCreate({e});
        _editorContext.EndUndo();
    }

    ImGui::Separator();
    ImGui::Text("Scene Entities:");
    
    // Display all entities in the registry (except the scene entity itself)
    auto& registry = _scene->GetRegistry();
    auto view = registry.view<Core::UUID>();

    std::vector<entt::entity> entities(view.begin(), view.end());
    std::sort(entities.begin(), entities.end(), [&registry](entt::entity a, entt::entity b) {
        const auto& uuidA = registry.get<Core::UUID>(a);
        const auto& uuidB = registry.get<Core::UUID>(b);
        return uuidA.value < uuidB.value;
    });

    // Get selection from EditorContext
    auto& selectedEntities = _editorContext.GetSelectedEntities();
    
	bool deleteEntitiesPressed = false;
    for (size_t i = 0; i < entities.size(); ++i) {
        auto entity = entities[i];
        const auto& nameComp = registry.try_get<NameComponent>(entity);
        
        // Skip the scene entity
        if (entity == _scene->GetSceneEntity())
            continue;
        
        // Display entity with its name
        ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
        
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
            
            if (isShiftDown && _lastClickedEntity != entt::null) {
                // Shift+Click: Select range from last clicked to current
                // Find indices of last clicked and current entity
                auto lastIt = std::find(entities.begin(), entities.end(), _lastClickedEntity);
                auto currentIt = entities.begin() + i;
                
                if (lastIt != entities.end()) {
                    // Determine range direction
                    auto startIt = (lastIt < currentIt) ? lastIt : currentIt;
                    auto endIt = (lastIt < currentIt) ? currentIt : lastIt;
                    
                    // Build new selection
                    std::set<entt::entity> newSelection;
                    
                    // Keep existing selection if holding ctrl
                    if (isCtrlDown) {
                        newSelection = selectedEntities;
                    }
                    
                    // Select all entities in range
                    for (auto it = startIt; it <= endIt; ++it) {
                        if (*it != _scene->GetSceneEntity()) {
                            newSelection.insert(*it);
                        }
                    }
                    
                    _editorContext.SetSelection(newSelection);
                }
            }
            else if (isCtrlDown) {
                // Ctrl+Click: Toggle selection
                std::set<entt::entity> newSelection = selectedEntities;
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
                _editorContext.SetSelection({entity});
                _lastClickedEntity = entity;
            }
        }
        
        // Handle delete key - check once per frame, not per entity
        if (!ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
        {
            deleteEntitiesPressed = true;
        }

        // Show entity ID in tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Entity ID: %u", entt::to_integral(entity));
            
            // Show UUID if present
            if (auto* uuid = registry.try_get<Core::UUID>(entity)) {
                ImGui::Text("UUID: %llu", uuid->value);
            }

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
            _lastClickedEntity = entt::null;
        }
	}

    ImGui::End();
}