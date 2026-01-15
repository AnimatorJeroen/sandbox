#pragma once
#include "core/memory/SelectionArchive.h"
#include "../vendor/include/entt/entt.hpp"
#include "IOp.h"
#include <unordered_map>

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

    // Helper: Create new entities and build a remap table from old IDs to new IDs
    inline std::unordered_map<entt::entity, entt::entity> 
    recreate_entities(entt::registry& reg, const std::vector<entt::entity>& old_entities) {
        std::unordered_map<entt::entity, entt::entity> remap;
        remap.reserve(old_entities.size());
        
        for (auto old_e : old_entities) {
            entt::entity new_e = reg.create();
            remap[old_e] = new_e;
        }
        
        return remap;
    }

    // Helper: Restore a set of components using the remap table
    template<typename C>
    void restore_component_set(
        entt::registry& reg,
        const std::vector<std::pair<entt::entity, C>>& components,
        const std::unordered_map<entt::entity, entt::entity>& remap)
    {
        for (const auto& [old_e, component] : components) {
            auto it = remap.find(old_e);
            if (it != remap.end()) {
                entt::entity new_e = it->second;
                reg.emplace<C>(new_e, component);
            }
        }
    }

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
            archive = make_selection_snapshot<Cs...>(reg, entities_to_create);

            // Store entity list
            createdIds.reserve(entities_to_create.size());
            for (auto e : entities_to_create) {
                createdIds.push_back(_reg.get<UUID>(e));
            }
        }

        void Apply() override {
            // Create new entities and build old->new map
            auto remap = recreate_entities(_reg, archive.entities);

            // Restore all components using fold expression over the tuple
            restore_all_components(remap, std::index_sequence_for<Cs...>{});

        }

        void Revert() override {
            // Destroy all created entities
            for (auto id : createdIds) {
                // Find entity by UUID and destroy it
                auto view = _reg.view<UUID>();
                for (auto e : view) {
                    if (_reg.get<UUID>(e).value == id.value) {
                        _reg.destroy(e);
                        break;
                    }
                }
            }
        }

    private:
        // Helper to restore all component types in the archive
        template<std::size_t... Is>
        void restore_all_components(
            const std::unordered_map<entt::entity, entt::entity>& remap,
            std::index_sequence<Is...>)
        {
            // Fold expression to call restore_component_set for each component type
            (restore_component_set(_reg, std::get<Is>(archive.storages), remap), ...);
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
            archive = make_selection_snapshot<Cs...>(reg, entities_to_destroy);
            
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
                auto view = _reg.view<UUID>();
                for (auto e : view) {
                    if (_reg.get<UUID>(e).value == id.value) {
                        _reg.destroy(e);
                        break;
                    }
                }
            }
        }

        void Revert() override {
            // Recreate entities from snapshot
            auto remap = recreate_entities(_reg, archive.entities);

            // Restore all components
            restore_all_components(remap, std::index_sequence_for<Cs...>{});

        }

    private:
        // Helper to restore all component types in the archive
        template<std::size_t... Is>
        void restore_all_components(
            const std::unordered_map<entt::entity, entt::entity>& remap,
            std::index_sequence<Is...>)
        {
            (restore_component_set(_reg, std::get<Is>(archive.storages), remap), ...);
        }
    };

} // namespace Core
