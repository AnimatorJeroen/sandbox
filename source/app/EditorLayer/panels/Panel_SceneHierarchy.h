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
	
	// Prints all components present on an entity to the console
	void PrintEntityComponents(Entity entity);
	
private:
	void RenderEntityNode(Entity entity, const std::set<Entity>& selectedEntities, bool& deleteEntitiesPressed);
	void HandleDragDropSource(Entity entity);
	void HandleDragDrop(Entity entity);
	void HandleDragDropAsParent(Entity entity, float relativeY, float itemHeight);
	void HandleDragDropBetween(Entity entity, float relativeY, float itemHeight);
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
	
	// Track all entities in render order for shift-select
	std::vector<Entity> _entitiesInRenderOrder;
	
	// Pending shift-select operation (processed after all entities are rendered)
	Entity _pendingShiftSelectEntity = Entity::Null();
	std::set<Entity> _pendingShiftSelectBaseSelection;
};