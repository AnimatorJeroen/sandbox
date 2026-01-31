# Scene Serialization Using SelectionArchive

## Overview

The Scene class uses `SelectionArchive` for binary serialization instead of cereal. This provides:
- **Consistency**: Same system used for clipboard, undo/redo, and scene save/load
- **Simplicity**: No manual component collection loops - full registry snapshot
- **Type-safety**: Component types managed through `AppComponentTypes` tuple
- **Robustness**: Creates new entity IDs on load to avoid conflicts

## File Format

Scene files (`.scene`) use the following binary format:
[Version: uint32_t]
[Entity Count: uint64_t]
[Entity IDs: entt::entity[]] (stored but remapped on load)
[For each component type in AppComponentTypes:]
  - Component Count: uint64_t
  - [For each component:]
    - Entity ID: entt::entity (remapped on load)
    - Component Data: raw bytes
**Note**: The entire registry is saved, including the scene root entity with its `SceneData` component. No separate metadata section needed!

## Usage

### Saving a Scene
scene->SaveToFile("path/to/scene.scene");
Before saving, camera settings are synced to the `SceneData` component on the scene root entity.

### Loading a Scene
scene->LoadFromFile("path/to/scene.scene");
The scene loading process:
1. Clear the entire registry
2. Restore all entities from the archive (including scene root)
3. Find the scene entity by looking for the `SceneData` component
4. Extract camera settings from `SceneData` and apply them

**Entity IDs are NOT preserved** - new entity IDs are generated on load. This prevents conflicts and avoids "gap" entities.

## What Gets Preserved

? **Preserved:**
- All components from `AppComponentTypes` (including `SceneData`)
- Scene metadata (name, color) via `SceneData` component
- Camera settings (position, target, FOV) via `SceneData` component
- Entity-to-component relationships
- UUIDs (as components)

? **NOT Preserved:**
- Entity IDs (remapped to new IDs)
- Entity references within components (would become invalid)

## Scene Root Entity

The scene root entity is special:
- **Marker**: Has a `SceneData` component (no other entities have this)
- **Metadata**: Stores scene name, color
- **Camera**: Stores camera position, target, FOV
- **After Load**: The scene finds this entity by querying for `SceneData`
// After loading, find the scene entity
auto sceneDataView = _registry.view<SceneData>();
_sceneEntity = sceneDataView.front();

// Extract camera settings
const auto& sceneData = _registry.get<SceneData>(_sceneEntity);
m_CameraPosition = sceneData.cameraPosition;
m_CameraTarget = sceneData.cameraTarget;
m_CameraFOV = sceneData.cameraFOV;
## Adding New Component Types

To add a new component type to scene serialization:

1. Define your component struct in `Components.h`
2. Add it to the `AppComponentTypes` tuple
3. That's it! The serialization system will automatically include it

Example:struct MyNewComponent {
    int value;
    float data;
};

using AppComponentTypes = std::tuple<
    Core::UUID,
    Transform,
    NameComponent,
    SceneData,
    MyNewComponent  // Added here
>;
## Technical Details

### Key Design Decisions

1. **Full Registry Snapshot**: `MakeFullSnapshot` captures everything, including scene entity
2. **Component-Based Identification**: Find scene entity by `SceneData` component, not by ID
3. **No Separate Metadata**: Camera and scene settings stored in `SceneData` component
4. **Automatic Discovery**: Scene entity is found after restore, not tracked by ID

### Scene Loading Process

1. **Clear Registry** - `_registry.clear()` removes all entities
2. **Load Archive** - Read entity IDs and component data from file
3. **Remap Entities** - `RestoreEntitiesAndComponents` creates new entities and remaps all component data
4. **Find Scene Entity** - Query for entity with `SceneData` component
5. **Restore Camera** - Extract and apply camera settings from `SceneData`

### Scene vs. Clipboard Behavior

| Feature | Scene Save/Load | Clipboard |
|---------|----------------|-----------|
| Entity IDs | **Remapped (new IDs)** | Remapped (new IDs) |
| Selection | All entities | Selected only |
| UUIDs | Preserved as component | Regenerated |
| SceneData | Included in snapshot | Not included |
| Scene Entity | Found via component query | N/A |

### Why This Approach?

**Benefits:**
- ? **Simpler**: No special-case code for scene entity
- ? **Robust**: Scene entity automatically included in snapshot
- ? **Flexible**: Camera settings naturally part of serialized data
- ? **Discoverable**: Scene entity found by component, not hardcoded ID

**Previous Approach Issues:**
- ? Separate metadata save/load methods
- ? Hardcoded scene entity ID preservation
- ? Fragile special-case handling

## Migration from Cereal

Old scene files using cereal cannot be loaded with this new system. The file formats are incompatible.

## Limitations

- **POD Components Only**: Components must be trivially copyable (no pointers, no virtual functions)
- **No Entity Reference Preservation**: If components store entity IDs, they become invalid on load
- **Binary Format**: Files are not human-readable or easily diffable
- **Single Scene Entity**: Assumes only one entity has `SceneData` component

## Best Practices

1. **Use UUIDs for References**: Store `Core::UUID` components instead of `entt::entity` for entity references
2. **Keep SceneData Unique**: Only the scene root should have `SceneData` component
3. **Sync Camera Changes**: Call `SetCameraPosition`/`SetCameraTarget` to keep `SceneData` in sync
4. **Post-Load Fixup**: If you need to resolve entity references, do it after loading by UUID lookup
5. **Version Your Files**: The version field allows future format changes

## Example: Custom Scene Metadata

To add custom scene-level metadata, extend `SceneData`:
struct SceneData {
    float sceneColor = 0.f;
    String64 _name;
    
    // Camera settings
    glm::vec3 cameraPosition{0.0f, 5.0f, 10.0f};
    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};
    float cameraFOV = 45.0f;
    
    // Your custom metadata
    String64 author;
    float timeOfDay = 12.0f;
    bool fogEnabled = false;
};
No changes needed in save/load code - it's all automatic!

## Future Improvements

Potential enhancements:

1. **Entity Reference Remapping**: Automatically update entity references using a remap table
2. **UUID-based References**: Use UUIDs instead of entity IDs in components
3. **Compression**: Add optional compression for smaller files
4. **Incremental Saves**: Only save changed entities
5. **Text Format Option**: JSON or XML alternative for version control
6. **Schema Versioning**: Handle component changes gracefully
