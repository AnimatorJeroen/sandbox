#pragma once
#include <memory>
#include <set>
#include "app/sceneLayer/Entity.h"

class Scene;
class EditorContext;

namespace Core {
	template<typename T1, typename T2>
	class Applicator;
}

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene, EditorContext& editorContext);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
private:
	void RenderEntityNode(Entity entity, const std::set<Entity>& selectedEntities, bool& deleteEntitiesPressed);
	void HandleDragDropSource(Entity entity);
	void HandleDragDropTarget(Entity entity);
	void HandleDragDropBetween(Entity entity);
	void HandleEmptySpaceDrop(const std::vector<Entity>& rootEntities);
	
	Scene* _scene;
	EditorContext& _editorContext;
	Entity _lastClickedEntity;
	
	// Drag-drop visual feedback
	enum class DropLocation { None, OnEntity, BeforeEntity, AfterEntity };
	Entity _dropTargetEntity = Entity::Null();
	DropLocation _dropLocation = DropLocation::None;
	
	// Track last rendered entity for empty space drop detection
	Entity _lastRenderedEntity = Entity::Null();
	ImVec2 _lastEntityRectMax = ImVec2(0, 0);
	
	// Track entities that should be expanded next frame (after drop)
	std::set<uint64_t> _entitiesToExpand;
};