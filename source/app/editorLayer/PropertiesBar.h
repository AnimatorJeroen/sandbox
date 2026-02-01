#pragma once
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

class Scene;
class EditorContext;

class PropertiesBar
{
private:
    Scene* _scene = nullptr;
    EditorContext& _editorContext;
    
public:
    explicit PropertiesBar(EditorContext& editorContext)
        : _editorContext(editorContext) {}

    void Render();

    void SetContext(Scene& scene);

};
