#pragma once
#include "../vendor/include/entt/entt.hpp"
#include <unordered_set>
#include <vector>
#include <tuple>
#include <unordered_map>

namespace Core {

    // Generic SelectionArchive that can capture any set of component types
    template<class... Cs>
    struct SelectionArchive {
        // Default constructor for empty archive
        SelectionArchive() = default;

        // Constructor with selection set (for capturing from registry)
        explicit SelectionArchive(const std::unordered_set<entt::entity>& set)
            : sel(&set) {
        }

        // Move constructor
        SelectionArchive(SelectionArchive&&) = default;
        SelectionArchive& operator=(SelectionArchive&&) = default;

        // Delete copy (large data)
        SelectionArchive(const SelectionArchive&) = delete;
        SelectionArchive& operator=(const SelectionArchive&) = delete;

        const std::unordered_set<entt::entity>* sel{}; // not owning (nullptr when standalone)

        // Store captured entities (original ids)
        std::vector<entt::entity> entities;

        // Per-component captured payloads
        std::tuple<std::vector<std::pair<entt::entity, Cs>>...> storages;

        // Entities sink for entt::snapshot - accepts underlying type
        void operator()(std::underlying_type_t<entt::entity> e) {
            entt::entity entity{e};
            if (!sel || sel->count(entity)) {
                entities.push_back(entity);
            }
        }
        
        // Entities sink for entt::snapshot - accepts entity type
        void operator()(entt::entity e) {
            if (!sel || sel->count(e)) {
                entities.push_back(e);
            }
        }

        // Generic component capture
        template<class C>
        void capture(entt::entity e, const C& c) {
            if (!sel || sel->count(e)) {
                auto& vec = std::get<std::vector<std::pair<entt::entity, C>>>(storages);
                vec.emplace_back(e, c); // copy-by-value
            }
        }

        // Component sink for entt::snapshot - with underlying entity type
        template<class C>
        void operator()(std::underlying_type_t<entt::entity> e, const C& c) {
            capture<C>(entt::entity{e}, c);
        }

        // Component sink for entt::snapshot - with entity type
        template<class C>
        void operator()(entt::entity e, const C& c) {
            capture<C>(e, c);
        }
    };

    namespace ArchiveHelpers
    {
        // Helper to create a snapshot of selected entities with specified components
        template<class... Cs>
        SelectionArchive<Cs...> MakeSnapshot(
            entt::registry& reg,
            const std::unordered_set<entt::entity>& sel)
        {
            SelectionArchive<Cs...> archive{ sel };

            // Manually iterate and capture entities with their components
            for (auto entity : sel) {
                if (reg.valid(entity)) {
                    archive.entities.push_back(entity);

                    // Capture each component type
                    ((reg.all_of<Cs>(entity) ? archive.template capture<Cs>(entity, reg.get<Cs>(entity)) : void()), ...);
                }
            }

            return archive;
        }

        // Helper: Restore a single component type using the remap table
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

        // Unified helper: Restore entities and all their components from a SelectionArchive
        // Creates new entities, builds remap table, and restores all component types
		// Returns the new entities created
        template<class... Cs>
        std::unordered_set<entt::entity> RestoreEntitiesAndComponents(
            entt::registry& reg,
            const SelectionArchive<Cs...>& archive)
        {
            // Create new entities and build remap table
            std::unordered_map<entt::entity, entt::entity> remap;
            remap.reserve(archive.entities.size());

            for (auto old_e : archive.entities) {
                entt::entity new_e = reg.create();
                remap[old_e] = new_e;
            }

            // Restore all component types using fold expression
            auto restore_all = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                (restore_component_set(reg, std::get<Is>(archive.storages), remap), ...);
            };
            restore_all(std::index_sequence_for<Cs...>{});

            // Collect the new entities
            std::unordered_set<entt::entity> newEntities;
            for (const auto& [oldEntity, newEntity] : remap) {
                newEntities.insert(newEntity);
            }

            return newEntities;
        }
    }

} // namespace Core

