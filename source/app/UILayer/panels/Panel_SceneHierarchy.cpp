#include "Panel_SceneHierarchy.h"
#include <imgui/imgui.h>

Panel_SceneHierarchy::Panel_SceneHierarchy(Scene& context) : _scene(context)
{
}

void Panel_SceneHierarchy::OnRender()
{
	static char inputTextbuffer[128];
    ImGui::Begin("Scene Hierarchy");
	ImGui::InputText("input field", inputTextbuffer, sizeof(inputTextbuffer));
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
        _scene.shapes.push_back(newCircle);
        posIncr += 20;
    }

    for(const auto& entity : _scene.shapes)
    {
        ImGui::Text("Entity ID: %d");
	}
    ImGui::End();
}