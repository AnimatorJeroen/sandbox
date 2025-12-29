#include "Panel_SceneHierarchy.h"
#include "app/sceneLayer/SceneManager.h"
#include <imgui/imgui.h>

Panel_SceneHierarchy::Panel_SceneHierarchy(const SceneManager& sceneManager) : _sceneManager(sceneManager)
{
}

void Panel_SceneHierarchy::Render()
{
    static char inputTextbuffer[128];
    static std::shared_ptr<Scene> lastScene = nullptr;

    auto scene = _sceneManager.GetActiveScene();

    if (scene != lastScene)
    {
        const std::string& sceneName = scene->GetName();
        strncpy_s(inputTextbuffer, sceneName.c_str(), sizeof(inputTextbuffer) - 1);
        inputTextbuffer[sizeof(inputTextbuffer) - 1] = '\0';
        lastScene = scene;
    }
    
    ImGui::Begin("Scene Hierarchy");
	
	// Update scene name when input changes
	if (ImGui::InputText("input field", inputTextbuffer, sizeof(inputTextbuffer)))
	{
		scene->SetName(std::string(inputTextbuffer));
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
        scene->GetShapes().push_back(newCircle);
        posIncr += 20;
    }

    for(const auto& entity : scene->GetShapes())
    {
        ImGui::Text("Entity ID: %d");
	}
    ImGui::End();
}