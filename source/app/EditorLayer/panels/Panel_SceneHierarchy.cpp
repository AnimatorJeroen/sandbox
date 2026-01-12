#include "Panel_SceneHierarchy.h"
#include <imgui/imgui.h>
#include <app/sceneLayer/shape/Circle.h>
#include <core/memory/SelectionArchive.h>

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
        auto& reg = _scene->GetRegistry();
         auto e1 = reg.create();
         reg.emplace<NameComponent>(e1, "Entity1");

         // Create a snapshot of entities to use as a template
         std::unordered_set<entt::entity> selection = {e1};

        _applicator.BeginUndo();
        _applicator.CreateAuto(selection);

        reg.remove<NameComponent>(e1);

        _applicator.EndUndo();
    }

    ImGui::Separator();
    ImGui::Text("Scene Entities:");
    
    // Display all entities in the registry (except the scene entity itself)
    auto& registry = _scene->GetRegistry();
    auto view = registry.view<NameComponent>();
    
    for (auto entity : view) {
        const auto& nameComp = view.get<NameComponent>(entity);
        
        // Skip the scene entity
        if (entity == _scene->GetSceneEntity())
            continue;
        
        // Display entity with its name
        ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
        
        bool nodeOpen = ImGui::TreeNodeEx(
            nameComp.name.data, 
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
        );
        
        // Show entity ID in tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Entity ID: %u", entt::to_integral(entity));
            
            // Show component info
            if (auto* dummy = registry.try_get<DummyComponent>(entity)) {
                ImGui::Text("Value: %.2f", dummy->value);
            }
            ImGui::EndTooltip();
        }
        
        ImGui::PopID();
    }

    ImGui::End();
}