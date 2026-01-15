#include "Panel_SceneHierarchy.h"
#include <imgui/imgui.h>
#include <app/sceneLayer/shape/Circle.h>
#include <core/memory/SelectionArchive.h>
#include <core/UUID.h>

Panel_SceneHierarchy::Panel_SceneHierarchy(Scene& scene, Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator) : _scene(&scene), _applicator(applicator)
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
        _applicator.BeginUndo();
        _applicator.SetField(_scene->GetSceneEntity(), "Scene.sceneColor", sceneColor);
        _applicator.EndUndo();
    }
	// Update scene name when input changes
    strncpy_s(inputTextbuffer, _scene->GetName().data, sizeof(inputTextbuffer) - 1);
    ImGui::InputText("input field", inputTextbuffer, sizeof(inputTextbuffer));
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        //_scene->SetName(std::string(inputTextbuffer));
        _applicator.BeginUndo();
        _applicator.SetField(_scene->GetSceneEntity(), "Scene.name", String64(inputTextbuffer));
        _applicator.EndUndo();
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
        _applicator.BeginUndo();
        auto e = _scene->CreateEntity();
        _applicator.CaptureCreate({e});
        _applicator.EndUndo();
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

    entt::entity entityToDelete = entt::null;
    for (auto entity : entities) {
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
        if (_selectedEntity == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        bool nodeOpen = ImGui::TreeNodeEx(
            nameComp ? nameComp->name.data : "Unnamed",
            flags
        );
        
        // Handle selection
        if (ImGui::IsItemClicked()) {
            _selectedEntity = entity;
        }
        
        // Handle delete key
        if (_selectedEntity == entity && ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
        {
            entityToDelete = entity;
        }

        // Show entity ID in tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Entity ID: %u", entt::to_integral(entity));
            
            // Show UUID if present
            if (auto* uuid = registry.try_get<Core::UUID>(entity)) {
                ImGui::Text("UUID: %llu", uuid->value);
            }
            
            // Show component info
            if (auto* dummy = registry.try_get<DummyComponent>(entity)) {
                ImGui::Text("Value: %.2f", dummy->value);
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

    if(entityToDelete != entt::null)
    {
        _applicator.BeginUndo();
        _applicator.CaptureDelete({ entityToDelete });
        _applicator.EndUndo();
        
        // Clear selection if we deleted the selected entity
        if (_selectedEntity == entityToDelete) {
            _selectedEntity = entt::null;
        }
	}

    ImGui::End();
}