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
	void RenderFBXBoneComponent(Entity entity);
	void RenderFBXAnimationChannelsComponent(Entity entity);
	void RenderAddComponentButton(Entity entity);
	
	// Generic helper to add a component with undo support
	template<typename ComponentType, typename... Args>
	void AddComponentMenuItem(Entity entity, const char* menuLabel, Args&&... args)
	{
		bool hasComponent = entity.HasComponent<ComponentType>();
		
		// Show menu item as disabled if component already exists
		if (hasComponent)
		{
			ImGui::BeginDisabled();
		}
		
		if (ImGui::MenuItem(menuLabel, nullptr, false, !hasComponent))
		{
			auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, ComponentType>(
				_scene->GetRegistry(), { entity.GetHandle() });

			entity.AddComponent<ComponentType>(std::forward<Args>(args)...);

			auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, ComponentType>(
				_scene->GetRegistry(), { entity.GetHandle() });

			_editorContext.BeginUndo();
			_editorContext.applicator().CaptureComponentChange<Core::UUID, ComponentType>(
				{ entity.GetHandle() }, std::move(before), std::move(after));
			_editorContext.EndUndo();

			ImGui::CloseCurrentPopup();
		}
		
		if (hasComponent)
		{
			ImGui::EndDisabled();
		}
	}
	
	Scene* _scene;
	EditorContext& _editorContext;
};
