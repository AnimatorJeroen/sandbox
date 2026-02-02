#pragma once
#include "core/applicator/SelectionArchive.h"
#include <entt/entt.hpp>
#include "IOp.h"
#include <unordered_map>
#include "core/UUID.h"

// ===========================================================================
// CaptureCreateOp and CaptureDeleteOp
// ===========================================================================
//
// USAGE EXAMPLE:
//
// // Define your component types
// struct Transform { Vec3 position; Vec3 scale; };
// struct Name { std::string value; };
//
// // Create entities with components
// entt::registry registry;
// auto e1 = registry.create();
// registry.emplace<Core::UUID>(e1);  // Always add UUID as base component
// registry.emplace<Transform>(e1, Vec3{1,2,3}, Vec3{1,1,1});
// registry.emplace<Name>(e1, "Entity1");
//
// // Create a snapshot of entities to use as a template
// std::unordered_set<entt::entity> selection = {e1};
// auto snapshot = Core::make_selection_snapshot<Core::UUID, Transform, Name>(registry, selection);
//
// // Create op for undo/redo (IMPORTANT: Include Core::UUID in template parameters)
// auto createOp = std::make_unique<Core::CaptureCreateOp<Core::UUID, Transform, Name>>(
//     registry, std::move(snapshot));
//
// // Apply: creates new entities
// createOp->Apply();
// // Now createOp->created contains the new entities
//
// // Revert: destroys the created entities
// createOp->Revert();
//
// // Apply again: recreates them (redo)
// createOp->Apply();
//
// // For destroying entities:
// std::unordered_set<entt::entity> to_destroy = {e1};
// auto destroyOp = std::make_unique<Core::CaptureDeleteOp<Core::UUID, Transform, Name>>(
//     registry, to_destroy);
//
// // Apply: destroys entities (snapshot already taken in constructor)
// destroyOp->Apply();
//
// // Revert: recreates entities from snapshot
// destroyOp->Revert();
//
// ===========================================================================

namespace Core {

    // CaptureCreateOp: Captures entity creation for undo/redo
    template<class... Cs>
    class CaptureCreateOp : public IOp {
    public:
        SelectionArchive<Cs...> archive;    // OWNED snapshot (e.g., prefab data)
        std::vector<UUID> createdIds;  // filled on Apply
        entt::registry& _reg;

        explicit CaptureCreateOp(entt::registry& reg) : _reg(reg) {}

        // Constructor that takes an archive (for creating from snapshot)
        CaptureCreateOp(entt::registry& reg, const std::unordered_set<entt::entity>& entities_to_create)
            : _reg(reg) {
        
            // create snapshot of current selection
            archive = ArchiveHelpers::MakeSnapshot<Cs...>(reg, entities_to_create);

            // Store entity list
            createdIds.reserve(entities_to_create.size());
            for (auto e : entities_to_create) {
                createdIds.push_back(_reg.get<UUID>(e));
            }
        }

        void Apply() override {
            // Use unified restore function
            ArchiveHelpers::RestoreEntitiesAndComponents(_reg, archive);
        }

        void Revert() override {
            // Destroy all created entities
            for (auto id : createdIds) {
                for (auto e : _reg.view<UUID>()) {
                    if (_reg.get<UUID>(e).value == id.value) {
                        _reg.destroy(e);
                        break;
                    }
                }
            }
        }
    };

    // CaptureDeleteOp: Captures entity destruction for undo/redo
    // On Apply: destroys entities and saves snapshot
    // On Revert: recreates entities from snapshot
    template<class... Cs>
    class CaptureDeleteOp : public IOp {
    public:
        SelectionArchive<Cs...> archive;    // Snapshot taken before destruction
        std::vector<UUID> destroyedIds; // Entities to destroy
        entt::registry& _reg;

        explicit CaptureDeleteOp(entt::registry& reg) : _reg(reg) {}

        // Constructor with entities to destroy (takes snapshot on construction)
        CaptureDeleteOp(entt::registry& reg, const std::unordered_set<entt::entity>& entities_to_destroy)
            : _reg(reg) 
        {
            // Take snapshot before destruction
            archive = ArchiveHelpers::MakeSnapshot<Cs...>(reg, entities_to_destroy);
            
            // Store entity list
            destroyedIds.reserve(entities_to_destroy.size());
            for (auto e : entities_to_destroy) {
                destroyedIds.push_back(_reg.get<UUID>(e));
            }
        }

        void Apply() override {
            // Destroy all entities
            for (auto id : destroyedIds) {
                // Find entity by UUID and destroy it
                for (auto e : _reg.view<UUID>()) {
                    if (_reg.get<UUID>(e).value == id.value) {
                        _reg.destroy(e);
                        break;
                    }
                }
            }
        }

        void Revert() override {
            // Use unified restore function
            ArchiveHelpers::RestoreEntitiesAndComponents(_reg, archive);
        }
    };

    // ===========================================================================
    // CaptureComponentChangeOp
    // ===========================================================================
    //
    // USAGE EXAMPLE 1: Adding a component with undo/redo
    //
    // entt::registry registry;
    // auto entity = registry.create();
    // registry.emplace<Core::UUID>(entity);
    // registry.emplace<Transform>(entity, vec3{0,0,0}, vec3{1,1,1});
    // 
    // std::unordered_set<entt::entity> entities = {entity};
    // 
    // // Take "before" snapshot (without Mesh component)
    // auto beforeSnapshot = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Add the Mesh component
    // registry.emplace<Mesh>(entity, meshData);
    // 
    // // Take "after" snapshot (with Mesh component)
    // auto afterSnapshot = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Create undoable operation
    // auto op = std::make_unique<Core::CaptureComponentChangeOp<Core::UUID, Transform, Mesh>>(
    //     registry, entities, std::move(beforeSnapshot), std::move(afterSnapshot));
    // 
    // // op->Revert() - removes the Mesh component (back to "before" state)
    // // op->Apply()  - restores the Mesh component (back to "after" state)
    //
    // ===========================================================================
    //
    // USAGE EXAMPLE 2: Removing a component with undo/redo
    //
    // std::unordered_set<entt::entity> entities = {entity};
    // 
    // // Take "before" snapshot (with Mesh component)
    // auto beforeSnapshot = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Remove the Mesh component
    // registry.remove<Mesh>(entity);
    // 
    // // Take "after" snapshot (without Mesh component)
    // auto afterSnapshot = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Create undoable operation
    // auto op = std::make_unique<Core::CaptureComponentChangeOp<Core::UUID, Transform, Mesh>>(
    //     registry, entities, std::move(beforeSnapshot), std::move(afterSnapshot));
    // 
    // // op->Revert() - restores the Mesh component (back to "before" state)
    // // op->Apply()  - removes the Mesh component again (back to "after" state)
    //
    // ===========================================================================
    //
    // USAGE EXAMPLE 3: Using with Applicator (recommended)
    //
    // Core::Applicator<FieldTypes, ComponentTypes> applicator(undoManager);
    // 
    // std::unordered_set<entt::entity> entities = {entity};
    // 
    // // Take before snapshot
    // auto before = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Make changes (add/remove components)
    // registry.emplace<Mesh>(entity, meshData);
    // 
    // // Take after snapshot
    // auto after = Core::ArchiveHelpers::MakeSnapshot<Core::UUID, Transform, Mesh>(registry, entities);
    // 
    // // Capture the change
    // applicator.CaptureComponentChange<Core::UUID, Transform, Mesh>(
    //     entities, std::move(before), std::move(after));
    // 
    // // Now you can undo/redo
    // undoManager.Undo(); // Reverts to before state
    // undoManager.Redo(); // Goes back to after state
    //
    // ===========================================================================
    //
    // IMPORTANT NOTES:
    // 1. Always include Core::UUID in the template parameters (required for entity tracking)
    // 2. Include ALL component types that need to be tracked, not just the ones being changed
    // 3. The "before" snapshot should be taken BEFORE making any changes
    // 4. The "after" snapshot should be taken AFTER making all changes
    // 5. If adding multiple components, take both snapshots once (not per component)
    // 6. Use with BeginUndo()/EndUndo() to bundle multiple component changes into one undo step
    //
    // ===========================================================================

    /// <summary>
    /// CaptureComponentChangeOp: Captures component add/remove operations for undo/redo
    /// 
    /// This op stores two snapshots: before and after a component change.
    /// - For adding components: "before" has no component, "after" has the component
    /// - For removing components: "before" has the component, "after" doesn't
    /// - For bulk changes: can handle multiple entities and component types at once
    /// 
    /// Template parameters should include ALL component types that might be affected,
    /// not just the ones being added/removed. This ensures proper entity state restoration.
    /// </summary>
    template<class... Cs>
    class CaptureComponentChangeOp : public IOp {
    public:
        SelectionArchive<Cs...> beforeState;    // State before component change
        SelectionArchive<Cs...> afterState;     // State after component change
        std::vector<UUID> affectedEntityIds;    // UUIDs of affected entities
        entt::registry& _reg;

        explicit CaptureComponentChangeOp(entt::registry& reg) : _reg(reg) {}

        /// <summary>
        /// Constructor for component change operation
        /// </summary>
        /// <param name="reg">Registry containing the entities</param>
        /// <param name="entities">Set of entities being modified</param>
        /// <param name="before">Snapshot of entity state BEFORE the component change</param>
        /// <param name="after">Snapshot of entity state AFTER the component change</param>
        CaptureComponentChangeOp(
            entt::registry& reg,
            const std::unordered_set<entt::entity>& entities,
            SelectionArchive<Cs...>&& before,
            SelectionArchive<Cs...>&& after)
            : _reg(reg)
            , beforeState(std::move(before))
            , afterState(std::move(after))
        {
            // Store UUIDs of affected entities
            affectedEntityIds.reserve(entities.size());
            for (auto e : entities) {
                if (reg.valid(e) && reg.all_of<UUID>(e)) {
                    affectedEntityIds.push_back(reg.get<UUID>(e));
                }
            }
        }

        void Apply() override {
            // Restore the "after" state (with new/removed components)
            RestoreComponentState(afterState);
        }

        void Revert() override {
            // Restore the "before" state (original components)
            RestoreComponentState(beforeState);
        }

    private:
        /// <summary>
        /// Helper to restore component state from an archive
        /// Only restores components for entities that exist (by UUID lookup)
        /// </summary>
        void RestoreComponentState(const SelectionArchive<Cs...>& archive) {
            // Build a map from old entity IDs to current entity handles (via UUID)
            std::unordered_map<entt::entity, entt::entity> entityRemap;
            
            for (auto oldEntity : archive.entities) {
                // Find the corresponding component data for this entity
                UUID targetUuid;
                bool foundUuid = false;

                // Search for UUID in the archive's component data
                FindUuidInArchive(archive, oldEntity, targetUuid, foundUuid);

                if (foundUuid) {
                    // Find the current entity with this UUID
                    for (auto currentEntity : _reg.view<UUID>()) {
                        if (_reg.get<UUID>(currentEntity).value == targetUuid.value) {
                            entityRemap[oldEntity] = currentEntity;
                            break;
                        }
                    }
                }
            }

            // Now restore components for each remapped entity
            RestoreComponentsFromArchive(archive, entityRemap);
        }

        /// <summary>
        /// Find the UUID for a specific entity in the archive
        /// </summary>
        void FindUuidInArchive(
            const SelectionArchive<Cs...>& archive,
            entt::entity entity,
            UUID& outUuid,
            bool& found) const {
            
            // Use fold expression to search through all component storages
            found = (FindUuidInStorage<Cs>(archive, entity, outUuid) || ...);
        }

        /// <summary>
        /// Search a specific component storage for the entity's UUID
        /// </summary>
        template<typename C>
        bool FindUuidInStorage(
            const SelectionArchive<Cs...>& archive,
            entt::entity entity,
            UUID& outUuid) const {
            
            // Special handling for UUID component type
            if constexpr (std::is_same_v<C, UUID>) {
                const auto& storage = std::get<std::vector<std::pair<entt::entity, C>>>(archive.storages);
                for (const auto& [e, component] : storage) {
                    if (e == entity) {
                        outUuid = component;
                        return true;
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Restore all components from archive using the entity remap table
        /// </summary>
        void RestoreComponentsFromArchive(
            const SelectionArchive<Cs...>& archive,
            const std::unordered_map<entt::entity, entt::entity>& remap) {
            
            // For each entity in the remap, remove all tracked components first
            for (const auto& [oldEntity, currentEntity] : remap) {
                // Remove existing components (fold expression)
                (RemoveComponentIfExists<Cs>(currentEntity), ...);
            }

            // Then restore components from archive
            (RestoreComponentType<Cs>(archive, remap), ...);
        }

        /// <summary>
        /// Remove a component from an entity if it exists
        /// </summary>
        template<typename C>
        void RemoveComponentIfExists(entt::entity entity) {
            if (_reg.all_of<C>(entity)) {
                _reg.remove<C>(entity);
            }
        }

        /// <summary>
        /// Restore a specific component type from the archive
        /// </summary>
        template<typename C>
        void RestoreComponentType(
            const SelectionArchive<Cs...>& archive,
            const std::unordered_map<entt::entity, entt::entity>& remap) {
            
            const auto& storage = std::get<std::vector<std::pair<entt::entity, C>>>(archive.storages);
            
            for (const auto& [oldEntity, component] : storage) {
                auto it = remap.find(oldEntity);
                if (it != remap.end()) {
                    entt::entity currentEntity = it->second;
                    // Emplace or replace the component
                    _reg.emplace_or_replace<C>(currentEntity, component);
                }
            }
        }
    };

} // namespace Core
