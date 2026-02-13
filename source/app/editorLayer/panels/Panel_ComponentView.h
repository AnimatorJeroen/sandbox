#pragma once
#include <memory>
#include "app/sceneLayer/Entity.h"

class Scene;
class EditorContext;

class Panel_ComponentView
{
public:
	explicit Panel_ComponentView(Scene& scene, EditorContext& editorContext);
	~Panel_ComponentView() = default;
	void Render();
	void SetContext(Scene& scene);
	
private:
	void RenderComponentUI(Entity entity);
	void RenderTransformComponent(Entity entity);
	void RenderNameComponent(Entity entity);
	void RenderMeshComponent(Entity entity);
	void RenderCameraComponent(Entity entity);
	void RenderParentComponent(Entity entity);
	void RenderChildrenComponent(Entity entity);
	void RenderFBXComponents(Entity entity);
	void RenderAddComponentButton(Entity entity);
	
	Scene* _scene;
	EditorContext& _editorContext;
};
