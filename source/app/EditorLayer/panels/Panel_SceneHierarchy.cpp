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
    
    // Get selection from EditorContext
    auto& selectedEntities = _editorContext.GetSelectedEntities();
    
    // Build list of root entities (entities without parents)
    std::vector<Entity> rootEntities;
    for (const Entity& entity : entitiesSet) {
        // Skip the scene entity and camera entity
        if (entity == _scene->GetSceneEntity() || entity.HasComponent<CameraComponent>())
            continue;
            
        // Check if entity has no parent or parent is invalid
        if (!entity.HasComponent<Parent>() || !entity.GetComponent<Parent>().HasParent()) {
            rootEntities.push_back(entity);
        }
    }
    
    // Sort root entities by UUID
    std::sort(rootEntities.begin(), rootEntities.end(), [](const Entity& a, const Entity& b) {
        return a.UUID().value < b.UUID().value;
    });
    
    // Render the tree starting from root entities
	bool deleteEntitiesPressed = false;
    for (Entity entity : rootEntities) {
        RenderEntityNode(entity, selectedEntities, deleteEntitiesPressed);
    }
    
    // Allow dropping on empty space to unparent entities (make them root)
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY")) {
            // Get the dragged entity's UUID from payload
            uint64_t draggedUUID = *(const uint64_t*)payload->Data;
            
            // Find the dragged entity by UUID
            Entity draggedEntity = Entity::Null();
            auto view = _scene->GetRegistry().view<Core::UUID>();
            for (auto e : view) {
                if (_scene->GetRegistry().get<Core::UUID>(e).value == draggedUUID) {
                    draggedEntity = Entity(e, &_scene->GetRegistry());
                    break;
                }
            }
            
            // Remove parent component to make it a root entity
            if (draggedEntity && draggedEntity.HasComponent<Parent>()) {
                _editorContext.BeginUndo();
                
                // Get old parent and rebuild its children
                Parent& parentComp = draggedEntity.GetComponent<Parent>();
                if (parentComp.HasParent()) {
                    auto parentView = _scene->GetRegistry().view<Core::UUID>();
                    for (auto e : parentView) {
                        if (_scene->GetRegistry().get<Core::UUID>(e).value == parentComp.parentUUID.value) {
                            Entity oldParent(e, &_scene->GetRegistry());
                            parentComp.parentUUID = Core::UUID(0); // Clear parent
                            _scene->RebuildChildrenForEntity(oldParent);
                            break;
                        }
                    }
                } else {
                    parentComp.parentUUID = Core::UUID(0); // Clear parent
                }
                
                _editorContext.EndUndo();
            }
        }
        ImGui::EndDragDropTarget();
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

void Panel_SceneHierarchy::RenderEntityNode(Entity entity, const std::set<Entity>& selectedEntities, bool& deleteEntitiesPressed)
{
    const NameComponent* nameComp = entity.HasComponent<NameComponent>() 
        ? &entity.GetComponent<NameComponent>() 
        : nullptr;
    
    // Display entity with its name
    ImGui::PushID(static_cast<int>(entity.UUID().value));
    
    // Determine tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow 
                             | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    // Check if entity has children
    bool hasChildren = entity.HasComponent<Children>() && entity.GetComponent<Children>().HasChildren();
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    
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
        
        if (isCtrlDown) {
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
    
    // Handle delete key
    if (!ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        deleteEntitiesPressed = true;
    }

    // Handle drag-drop
    HandleDragDropSource(entity);
    HandleDragDropTarget(entity);

    // Show entity ID in tooltip
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Entity ID: %u", static_cast<uint32_t>(entt::to_integral(entity.GetHandle())));
        ImGui::Text("UUID: %llu", entity.UUID().value);
        if (selectedEntities.size() > 1) {
            ImGui::Text("(%zu entities selected)", selectedEntities.size());
        }
        ImGui::EndTooltip();
    }
    
    // Render children if node is open
    if (nodeOpen && hasChildren) {
        const Children& childrenComp = entity.GetComponent<Children>();
        
        // Convert children to Entity wrappers and sort by UUID
        std::vector<Entity> children;
        for (entt::entity childHandle : childrenComp.children) {
            Entity childEntity(childHandle, &_scene->GetRegistry());
            if (childEntity) { // Validate entity is still valid
                children.push_back(childEntity);
            }
        }
        
        std::sort(children.begin(), children.end(), [](const Entity& a, const Entity& b) {
            return a.UUID().value < b.UUID().value;
        });
        
        // Recursively render children
        for (Entity child : children) {
            RenderEntityNode(child, selectedEntities, deleteEntitiesPressed);
        }
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void Panel_SceneHierarchy::HandleDragDropSource(Entity entity)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        // Set payload to carry the entity's UUID
        uint64_t uuidValue = entity.UUID().value;
        ImGui::SetDragDropPayload("ENTITY_HIERARCHY", &uuidValue, sizeof(uint64_t));
        
        // Display preview
        const NameComponent* nameComp = entity.HasComponent<NameComponent>() 
            ? &entity.GetComponent<NameComponent>() 
            : nullptr;
        ImGui::Text("Moving: %s", nameComp ? nameComp->name.data : "Unnamed");
        
        ImGui::EndDragDropSource();
    }
}

void Panel_SceneHierarchy::HandleDragDropTarget(Entity entity)
{
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY")) {
            // Get the dragged entity's UUID from payload
            uint64_t draggedUUID = *(const uint64_t*)payload->Data;
            
            // Find the dragged entity by UUID
            Entity draggedEntity = Entity::Null();
            auto view = _scene->GetRegistry().view<Core::UUID>();
            for (auto e : view) {
                if (_scene->GetRegistry().get<Core::UUID>(e).value == draggedUUID) {
                    draggedEntity = Entity(e, &_scene->GetRegistry());
                    break;
                }
            }
            
            // Set parent relationship if entities are valid and not the same
            if (draggedEntity && entity && draggedEntity != entity) {
                // Prevent circular parent relationships
                bool isCircular = false;
                Entity checkParent = entity;
                while (checkParent.HasComponent<Parent>()) {
                    const Parent& parentComp = checkParent.GetComponent<Parent>();
                    if (!parentComp.HasParent()) break;
                    
                    // Find parent entity by UUID
                    auto parentView = _scene->GetRegistry().view<Core::UUID>();
                    Entity parentEntity = Entity::Null();
                    for (auto e : parentView) {
                        if (_scene->GetRegistry().get<Core::UUID>(e).value == parentComp.parentUUID.value) {
                            parentEntity = Entity(e, &_scene->GetRegistry());
                            break;
                        }
                    }
                    
                    if (parentEntity == draggedEntity) {
                        isCircular = true;
                        break;
                    }
                    
                    checkParent = parentEntity;
                    if (!checkParent) break;
                }
                
                if (!isCircular) {
                    _editorContext.BeginUndo();
                    _scene->SetParent(draggedEntity, entity);
                    _editorContext.EndUndo();
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}