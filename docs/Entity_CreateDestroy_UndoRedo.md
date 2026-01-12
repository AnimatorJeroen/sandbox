# Entity Creation/Destruction Undo/Redo System

## Overview

This implementation provides a complete, generic system for creating and destroying entity selections with full undo/redo support. The system is built on top of EnTT's snapshot capabilities and is designed to be reusable for various purposes including:

- Undo/Redo operations
- Copy/Paste clipboard
- Prefab instantiation
- Entity duplication

## Key Components

### 1. `SelectionArchive<Cs...>` (source/core/memory/SelectionArchive.h)

A generic template structure that captures a snapshot of entities and their components.

**Features:**
- Captures entities and their components using EnTT's snapshot system
- Fully generic - works with any combination of component types
- Supports both filtered (selection-based) and full registry snapshots
- Move-only semantics (deleted copy to avoid accidental large data copies)

**Usage:**
```cpp
// Capture specific entities
std::unordered_set<entt::entity> selection = {entity1, entity2};
auto archive = Core::make_selection_snapshot<Transform, Name>(registry, selection);

// Capture all entities
auto fullArchive = Core::make_full_snapshot<Transform, Name>(registry);
```

### 2. `CreateSelectionOp<Cs...>` (source/core/undo/ops/CreateOp.h)

An operation that creates entities from a snapshot.

**Behavior:**
- `Apply()`: Creates new entities and restores all components from the archive
- `Revert()`: Destroys all created entities

**Usage:**
```cpp
auto snapshot = Core::make_selection_snapshot<Transform, Name>(registry, selection);
auto createOp = std::make_unique<Core::CreateSelectionOp<Transform, Name>>(
    registry, std::move(snapshot));

createOp->Apply();   // Creates entities
// ... createOp->created contains the new entity IDs
createOp->Revert();  // Destroys entities
createOp->Apply();   // Recreates entities (redo)
```

### 3. `DestroySelectionOp<Cs...>` (source/core/undo/ops/CreateOp.h)

An operation that destroys entities with the ability to restore them.

**Behavior:**
- Constructor: Takes snapshot of entities before destruction
- `Apply()`: Destroys the entities
- `Revert()`: Recreates entities from the snapshot

**Usage:**
```cpp
std::unordered_set<entt::entity> to_destroy = {entity1, entity2};
auto destroyOp = std::make_unique<Core::DestroySelectionOp<Transform, Name>>(
    registry, to_destroy);

destroyOp->Apply();   // Destroys entities (snapshot already taken)
destroyOp->Revert();  // Recreates entities
```

## Implementation Details

### Helper Functions

1. **`recreate_entities()`**: Creates new entities and builds a remap table from old IDs to new IDs
2. **`restore_component_set()`**: Restores a specific component type to entities using the remap table

### Design Decisions

1. **Generic Component Types**: Uses template parameter packs (`class... Cs`) to support any combination of components
2. **Entity ID Remapping**: Original entity IDs from snapshots are remapped to new IDs on recreation
3. **Simple GUID System**: Currently doesn't implement GUID/StableId mapping (kept simple as requested)
4. **No Relationship Handling**: Entity relationships are not remapped (can be added later)

## Integration with Undo System

Both operations implement the `IOp` interface and can be used with the existing `UndoManager`:

```cpp
// In your UndoManager or command system
void CreateEntities(const std::unordered_set<entt::entity>& template_entities) {
    auto snapshot = Core::make_selection_snapshot<Transform, Name>(registry, template_entities);
    auto op = std::make_unique<Core::CreateSelectionOp<Transform, Name>>(
        registry, std::move(snapshot));
    
    op->Apply();
    
    // Add to undo stack
    undo_stack.push(std::move(op));
}
```

## Use Cases

### 1. Entity Creation with Undo
```cpp
// User creates entities in the scene
auto createOp = std::make_unique<CreateSelectionOp<Transform>>(registry, prefab_snapshot);
createOp->Apply();
undoManager.AddOperation(std::move(createOp));
```

### 2. Entity Deletion with Undo
```cpp
// User deletes selected entities
auto destroyOp = std::make_unique<DestroySelectionOp<Transform>>(registry, selected_entities);
destroyOp->Apply();
undoManager.AddOperation(std::move(destroyOp));
```

### 3. Copy/Paste
```cpp
// Copy: Take a snapshot
auto clipboard = Core::make_selection_snapshot<Transform, Name>(registry, selection);

// Paste: Create from snapshot
auto createOp = std::make_unique<CreateSelectionOp<Transform, Name>>(
    registry, std::move(clipboard));
createOp->Apply();
```

### 4. Prefab Instantiation
```cpp
// Store prefab as a snapshot
SelectionArchive<Transform, Mesh, Material> prefab = load_prefab("enemy.prefab");

// Instantiate multiple times
for (int i = 0; i < 10; ++i) {
    auto instance = prefab; // Would need to add copy support for this
    auto createOp = std::make_unique<CreateSelectionOp<Transform, Mesh, Material>>(
        registry, std::move(instance));
    createOp->Apply();
}
```

## Future Enhancements

1. **StableId/GUID Support**: Add automatic GUID generation and registration
2. **Relationship Remapping**: Support for parent-child hierarchies and entity references
3. **Partial Component Selection**: Allow capturing only specific components per entity
4. **Serialization**: Add serialization support for saving/loading snapshots to disk
5. **Copy Support**: Add optional copy constructor for prefab use cases

## Notes

- All components must be copyable (required by EnTT's snapshot system)
- Entity IDs change on recreation - don't rely on specific entity values
- The system is header-only and template-based for maximum flexibility
- No heap allocations during Apply/Revert (except for the initial snapshot)
