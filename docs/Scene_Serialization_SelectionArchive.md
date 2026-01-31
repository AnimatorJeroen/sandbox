# Scene Serialization Using SelectionArchive

## Overview

The Scene class now uses `SelectionArchive` for binary serialization instead of cereal. This provides:
- **Consistency**: Same system used for clipboard, undo/redo, and scene save/load
- **Simplicity**: No manual component collection loops
- **Type-safety**: Component types managed through `AppComponentTypes` tuple
- **Robustness**: Creates new entity IDs on load to avoid conflicts

## File Format

Scene files (`.scene`) use the following binary format:
[Version: uint32_t]
[SceneMetadata]
  - sceneColor: float
  - name: String64
  - cameraPosition: glm::vec3
  - cameraTarget: glm::vec3
  - cameraFOV: float
[Scene Entity ID: entt::entity] (stored but not used on load)
[Entity Count: uint64_t]
[Entity IDs: entt::entity[]] (stored but remapped on load)
[For each component type in AppComponentTypes:]
  - Component Count: uint64_t
  - [For each component:]
    - Entity ID: entt::entity (remapped on load)
    - Component Data: raw bytes
## Usage

### Saving a Scene
scene->SaveToFile("path/to/scene.scene");
### Loading a Scene
scene->LoadFromFile("path/to/scene.scene");
The scene will be completely cleared and restored from the file. **Note: Entity IDs are NOT preserved** - new entity IDs are generated on load. This prevents conflicts with the scene root entity and avoids "gap" entities.

## What Gets Preserved

? **Preserved:**
- All components from `AppComponentTypes`
- Scene metadata (name, color, camera settings)
- Entity-to-component relationships
- UUIDs (as components)

? **NOT Preserved:**
- Entity IDs (remapped to new IDs)
- Entity references within components (would become invalid)

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
    MyNewComponent  // Added here
>;
## Technical Details

### SelectionArchive Extensions

The following methods were added to `SelectionArchive`:

- `SaveToFile(filepath)` - Write archive to binary file
- `LoadFromFile(filepath)` - Read archive from binary file

Helper functions:

- `MakeFullSnapshot<Cs...>(registry)` - Capture ALL entities with specified components
- `RestoreEntitiesAndComponents<Cs...>(registry, archive)` - Restore entities with NEW IDs

### Scene Loading Process

1. **Clear Registry** - `_registry.clear()` removes all entities
2. **Recreate Scene Entity** - Create new scene root entity with metadata
3. **Load Archive** - Read entity IDs and component data from file
4. **Remap Entities** - `RestoreEntitiesAndComponents` creates new entities and remaps all component data
5. **Restore Camera** - Apply saved camera settings

### Scene vs. Clipboard Behavior

| Feature | Scene Save/Load | Clipboard |
|---------|----------------|-----------|
| Entity IDs | **Remapped (new IDs)** | Remapped (new IDs) |
| Selection | All entities | Selected only |
| UUIDs | Preserved as component | Regenerated |
| Metadata | Saved separately | Not saved |
| Scene Entity | Handled specially | Not included |

### Why Not Preserve Entity IDs?

The original `RestoreExactEntities` approach had fatal flaws:

1. **Gap Entities**: Creating entities in a loop (`while (reg.create() != target)`) leaves orphaned entities
2. **Scene Entity Collision**: The scene root entity gets recreated, potentially conflicting with loaded IDs
3. **Fragility**: Any ID conflict causes corruption
4. **Unnecessary**: Entity IDs are internal - users reference entities by UUID or hierarchy

**Solution**: Use `RestoreEntitiesAndComponents` which:
- Creates fresh entity IDs
- Maintains entity-to-component relationships
- Works with existing clipboard/paste code
- No special cases or workarounds needed

## Migration from Cereal

Old scene files using cereal cannot be loaded with this new system. The file formats are incompatible.

## Limitations

- **POD Components Only**: Components must be trivially copyable (no pointers, no virtual functions)
- **No Entity Reference Preservation**: If components store entity IDs, they become invalid on load
- **Binary Format**: Files are not human-readable or easily diffable
- **UUID Preservation**: UUIDs are preserved, but as regular components (not as entity identity)

## Best Practices

1. **Use UUIDs for References**: Store `Core::UUID` components instead of `entt::entity` for entity references
2. **Post-Load Fixup**: If you need to resolve entity references, do it after loading by UUID lookup
3. **Version Your Files**: The version field allows future format changes
4. **Test Save/Load**: Always test that your scenes can round-trip (save then load)

## Future Improvements

Potential enhancements:

1. **Entity Reference Remapping**: Automatically update entity references using a remap table
2. **UUID-based References**: Use UUIDs instead of entity IDs in components
3. **Compression**: Add optional compression for smaller files
4. **Incremental Saves**: Only save changed entities
5. **Text Format Option**: JSON or XML alternative for version control
6. **Schema Versioning**: Handle component changes gracefully
