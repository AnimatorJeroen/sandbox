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
    
    // Reset drop target state at the start of each frame
    _dropTargetEntity = Entity::Null();
    _dropLocation = DropLocation::None;
    _lastRenderedEntity = Entity::Null();
    _lastEntityRectMax = ImVec2(0, 0);
    
    // Clear and rebuild the render order list for shift-select
    _entitiesInRenderOrder.clear();
    
    // Reset pending shift-select
    _pendingShiftSelectEntity = Entity::Null();
    _pendingShiftSelectBaseSelection.clear();
    
    // Render the tree starting from root entities
	bool deleteEntitiesPressed = false;
    for (Entity entity : rootEntities) {
        RenderEntityNode(entity, selectedEntities, deleteEntitiesPressed);
    }
    
    // Process pending shift-select operation if any
    if (_pendingShiftSelectEntity && _lastClickedEntity) {
        int lastClickedIndex = -1;
        int currentIndex = -1;
        
        for (int i = 0; i < static_cast<int>(_entitiesInRenderOrder.size()); ++i) {
            if (_entitiesInRenderOrder[i] == _lastClickedEntity) {
                lastClickedIndex = i;
            }
            if (_entitiesInRenderOrder[i] == _pendingShiftSelectEntity) {
                currentIndex = i;
            }
        }
        
        // Select all entities in the range
        if (lastClickedIndex != -1 && currentIndex != -1) {
            std::set<Entity> newSelection = _pendingShiftSelectBaseSelection;
            int start = std::min(lastClickedIndex, currentIndex);
            int end = std::max(lastClickedIndex, currentIndex);
            
            for (int i = start; i <= end; ++i) {
                newSelection.insert(_entitiesInRenderOrder[i]);
            }
            
            _editorContext.SetSelection(newSelection);
        }
    }
    
    // Handle empty space drop (below last entity or in empty area)
    HandleEmptySpaceDrop(rootEntities);

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
    
    // Add this entity to the render order list for shift-select
    _entitiesInRenderOrder.push_back(entity);
    
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
    
    // Check if this entity should be expanded (after a drop operation)
    uint64_t entityUUID = entity.UUID().value;
    if (_entitiesToExpand.find(entityUUID) != _entitiesToExpand.end()) {
        ImGui::SetNextItemOpen(true);
        _entitiesToExpand.erase(entityUUID);
    }

    bool nodeOpen = ImGui::TreeNodeEx(
        nameComp ? nameComp->name.data : "Unnamed",
        flags
    );
    
    // Track this entity as the last rendered (for empty space drop detection)
    _lastRenderedEntity = entity;
    _lastEntityRectMax = ImGui::GetItemRectMax();
    
    // Handle selection
    if (ImGui::IsItemClicked()) {
        bool isShiftDown = ImGui::GetIO().KeyShift;
        bool isCtrlDown = ImGui::GetIO().KeyCtrl;
        
        if (isShiftDown && _lastClickedEntity) {
            // Shift+Click: Range selection from last clicked to current
            // Defer this operation until after all entities are rendered
            // so we have the complete _entitiesInRenderOrder list
            _pendingShiftSelectEntity = entity;
            _pendingShiftSelectBaseSelection = selectedEntities;
            // Don't update _lastClickedEntity on shift-click to allow extending the range
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
    
    // Handle delete key
    if (!ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        deleteEntitiesPressed = true;
    }

    // Handle drag-drop - centralized method
    HandleDragDropSource(entity);
    HandleDragDrop(entity);

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

void Panel_SceneHierarchy::HandleDragDrop(Entity entity)
{
    // Check if we're currently dragging an entity
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (!payload || !payload->IsDataType("ENTITY_HIERARCHY"))
        return;
    
    // Get the item rectangle and mouse position
    ImVec2 itemMin = ImGui::GetItemRectMin();
    ImVec2 itemMax = ImGui::GetItemRectMax();
    ImVec2 mousePos = ImGui::GetMousePos();
    
    // Check if mouse is hovering over this item
    if (mousePos.x < itemMin.x || mousePos.x > itemMax.x ||
        mousePos.y < itemMin.y || mousePos.y > itemMax.y)
        return;
    
    // Calculate relative position within item
    float itemHeight = itemMax.y - itemMin.y;
    float relativeY = mousePos.y - itemMin.y;
    
    // Register as a drag drop target - disable default rect since we draw custom feedback
    if (!ImGui::BeginDragDropTarget())
        return;
    
    // Determine drop zone based on mouse position and delegate
    // Top 35%: Drop before entity
    // Middle 30%: Drop as parent
    // Bottom 35%: Drop after entity
    if (relativeY < itemHeight * 0.35f) {
        // Top region - drop before
        HandleDragDropBetween(entity, relativeY, itemHeight);
        // Draw visual feedback line above entity if this is the drop target
        if (_dropTargetEntity == entity && _dropLocation == DropLocation::BeforeEntity) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = itemMin;
            drawList->AddLine(
                ImVec2(pos.x, pos.y),
                ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y),
                IM_COL32(255, 255, 0, 255), 2.0f
            );
        }
    }
    else if (relativeY > itemHeight * 0.65f) {
        // Bottom region - drop after
        HandleDragDropBetween(entity, relativeY, itemHeight);

        if (_dropTargetEntity == entity && _dropLocation == DropLocation::AfterEntity) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = itemMax;
            drawList->AddLine(
                ImVec2(itemMin.x, itemMax.y),
                ImVec2(itemMin.x + ImGui::GetContentRegionAvail().x, itemMax.y),
                IM_COL32(255, 255, 0, 255), 2.0f
            );
        }
    }
    else {
        // Middle region - drop as parent
        HandleDragDropAsParent(entity, relativeY, itemHeight);

        if (_dropTargetEntity == entity && _dropLocation == DropLocation::OnEntity) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRect(
                itemMin,
                itemMax,
                IM_COL32(255, 255, 0, 255), 2.0f
            );
        }
    }
    
    // End drag drop target - do this ONCE after delegating
    ImGui::EndDragDropTarget();
}

void Panel_SceneHierarchy::HandleDragDropAsParent(Entity entity, float relativeY, float itemHeight)
{
    // Check for hover without accepting to provide visual feedback
    if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
        if (payload->IsDataType("ENTITY_HIERARCHY")) {
            _dropTargetEntity = entity;
            _dropLocation = DropLocation::OnEntity;
        }
    }
    
    // Accept with flag to prevent ImGui from drawing its default highlight rect
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
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
                auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
                    _scene->GetRegistry(), { draggedEntity.GetHandle() });

                _scene->SetParent(draggedEntity, entity);

                auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
                    _scene->GetRegistry(), { draggedEntity.GetHandle() });

                _editorContext.BeginUndo();
                _editorContext.applicator().CaptureComponentChange<Core::UUID, Parent>(
                    { draggedEntity.GetHandle() }, std::move(before), std::move(after));
                _editorContext.EndUndo();
                
                // Expand the parent entity next frame to show the newly added child
                _entitiesToExpand.insert(entity.UUID().value);
            }
        }
    }
}

void Panel_SceneHierarchy::HandleDragDropBetween(Entity entity, float relativeY, float itemHeight)
{
    // Determine if we're in top or bottom region
    bool isTopRegion = relativeY < itemHeight * 0.35f;
    bool isBottomRegion = relativeY > itemHeight * 0.65f;
    
    if (!isTopRegion && !isBottomRegion)
        return;
    
    // Check for hover to provide visual feedback
    if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
        if (payload->IsDataType("ENTITY_HIERARCHY")) {
            // Set visual feedback based on region
            if (isTopRegion) {
                _dropTargetEntity = entity;
                _dropLocation = DropLocation::BeforeEntity;
            }
            else { // isBottomRegion
                _dropTargetEntity = entity;
                _dropLocation = DropLocation::AfterEntity;
            }
        }
    }
    
    // Accept with flag to prevent ImGui from drawing its default highlight rect
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
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
        
        if (draggedEntity && draggedEntity != entity) {
            // Get the parent of the target entity (if any)
            Entity targetParent = Entity::Null();
            if (entity.HasComponent<Parent>()) {
                const Parent& parentComp = entity.GetComponent<Parent>();
                if (parentComp.HasParent()) {
                    auto parentView = _scene->GetRegistry().view<Core::UUID>();
                    for (auto e : parentView) {
                        if (_scene->GetRegistry().get<Core::UUID>(e).value == parentComp.parentUUID.value) {
                            targetParent = Entity(e, &_scene->GetRegistry());
                            break;
                        }
                    }
                }
            }
            
            // Set same parent as target entity (or null if target is root)
            auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
                _scene->GetRegistry(), { draggedEntity.GetHandle() });

            _scene->SetParent(draggedEntity, targetParent);

            auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
                _scene->GetRegistry(), { draggedEntity.GetHandle() });

            _editorContext.BeginUndo();
            _editorContext.applicator().CaptureComponentChange<Core::UUID, Parent>(
                { draggedEntity.GetHandle() }, std::move(before), std::move(after));
            _editorContext.EndUndo();
        }
    }
}

void Panel_SceneHierarchy::HandleEmptySpaceDrop(const std::vector<Entity>& rootEntities)
{
    // Check if we're currently dragging an entity
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (!payload || !payload->IsDataType("ENTITY_HIERARCHY"))
        return;
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // Check if mouse is below the last rendered entity
    bool isBelowLastEntity = false;
    if (_lastRenderedEntity && _lastEntityRectMax.y > 0) {
        // Extend drop zone all the way to the bottom of the window
        if (mousePos.y > _lastEntityRectMax.y && 
            mousePos.x >= windowPos.x &&
            mousePos.x <= windowPos.x + windowSize.x &&
            mousePos.y <= windowPos.y + windowSize.y) {
            isBelowLastEntity = true;
        }
    }
    
    // Also handle case when there are no entities at all
    if (rootEntities.empty()) {
        // If empty and dragging, allow drop anywhere in window
        if (ImGui::IsWindowHovered()) {
            isBelowLastEntity = true;
            _lastRenderedEntity = Entity::Null();
        }
    }
    
    if (!isBelowLastEntity)
        return;
    
    // Set visual feedback for dropping after the last entity
    _dropTargetEntity = _lastRenderedEntity;
    _dropLocation = DropLocation::AfterEntity;
    
    // Draw the line below the last entity
    if (_lastRenderedEntity) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 lineStart(_lastEntityRectMax.x - ImGui::GetContentRegionAvail().x, _lastEntityRectMax.y);
        ImVec2 lineEnd(_lastEntityRectMax.x, _lastEntityRectMax.y);
        drawList->AddLine(lineStart, lineEnd, IM_COL32(255, 255, 0, 255), 2.0f);
    }
    
    // Accept the drop if mouse is released
    if (!ImGui::IsMouseReleased(0))
        return;
    
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
    
    if (!draggedEntity)
        return;
    
    // Drop below last entity means make it a root entity (null parent)
    auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
        _scene->GetRegistry(), { draggedEntity.GetHandle() });

    _scene->SetParent(draggedEntity, Entity::Null());

    auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Parent>(
        _scene->GetRegistry(), { draggedEntity.GetHandle() });

    _editorContext.BeginUndo();
    _editorContext.applicator().CaptureComponentChange<Core::UUID, Parent>(
        { draggedEntity.GetHandle() }, std::move(before), std::move(after));
    _editorContext.EndUndo();
}