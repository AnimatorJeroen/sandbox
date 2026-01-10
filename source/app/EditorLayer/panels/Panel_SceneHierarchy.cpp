#include "Panel_SceneHierarchy.h"
#include <imgui/imgui.h>
#include <app/sceneLayer/shape/Circle.h>


Panel_SceneHierarchy::Panel_SceneHierarchy(Scene& scene, Core::Applicator<AppValueTypes>& applicator) : _scene(&scene), _applicator(applicator)
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

    for(const auto& entity : _scene->GetShapes())
    {
        ImGui::Text("Entity ID: %d");
	}
    ImGui::End();
}