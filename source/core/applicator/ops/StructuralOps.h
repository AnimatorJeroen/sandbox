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

} // namespace Core
