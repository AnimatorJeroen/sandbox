#pragma once
#include <memory>
#include <functional>
#include "app/sceneLayer/Entity.h"

class Scene;
class EditorContext;

class Panel_ComponentView
{
public:
	explicit Panel_ComponentView(Scene& scene, EditorContext& editorContext);
	~Panel_ComponentView() = default;
	void Render(float height = 0.0f);
	void SetContext(Scene& scene);
	
private:
	void RenderComponentUI(Entity entity);
	void RenderTransformComponent(Entity entity);
	void RenderNameComponent(Entity entity);
	void RenderMeshComponent(Entity entity);
	void RenderCameraComponent(Entity entity);
	void RenderParentComponent(Entity entity);
	void RenderChildrenComponent(Entity entity);
	void RenderFBXSkeletonComponent(Entity entity);
	void RenderFBXSkinComponent(Entity entity);
	void RenderFBXAnimationComponent(Entity entity);
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
	
	// Generic helper to remove a component with undo support
	template<typename ComponentType>
	void RemoveComponentWithUndo(Entity entity)
	{
		if (!entity.HasComponent<ComponentType>())
			return;
		
		auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, ComponentType>(
			_scene->GetRegistry(), { entity.GetHandle() });
		
		entity.RemoveComponent<ComponentType>();
		
		auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, ComponentType>(
			_scene->GetRegistry(), { entity.GetHandle() });
		
		_editorContext.BeginUndo();
		_editorContext.applicator().CaptureComponentChange<Core::UUID, ComponentType>(
			{ entity.GetHandle() }, std::move(before), std::move(after));
		_editorContext.EndUndo();
	}
	
	// Generic helper to render component context menu
	template<typename ComponentType>
	void RenderComponentContextMenu(Entity entity, const char* componentName)
	{
		if (ImGui::BeginPopupContextItem(componentName))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				RemoveComponentWithUndo<ComponentType>(entity);
			}
			ImGui::EndPopup();
		}
	}

	template<typename ComponentType>
	void RenderComponent(Entity entity, std::function<void(Entity)> renderComponentImpl)
	{
		if (!entity.HasComponent<ComponentType>())
			return;
		const char* name = typeid(ComponentType).name();
		if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderComponentContextMenu<ComponentType>(entity, name);
			if (!entity.HasComponent<ComponentType>())
				return;
			renderComponentImpl(entity);
		}
		else
		{
			RenderComponentContextMenu<ComponentType>(entity, name);
		}
	}
	
	Scene* _scene;
	EditorContext& _editorContext;
};
