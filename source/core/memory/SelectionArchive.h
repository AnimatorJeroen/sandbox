#pragma once
#include "../vendor/include/entt/entt.hpp"
#include <unordered_set>
#include <vector>
#include <tuple>

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

    // Helper to create a snapshot of selected entities with specified components
    template<class... Cs>
    SelectionArchive<Cs...> make_selection_snapshot(
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

    // Helper to create a snapshot of selected entities with specified components (pointer overload)
    template<class... Cs>
    SelectionArchive<Cs...> make_selection_snapshot(
        entt::registry* reg,
        const std::unordered_set<entt::entity>& sel)
    {
        return make_selection_snapshot<Cs...>(*reg, sel);
    }

    // Helper to create a full snapshot of all entities with specified components
    template<class... Cs>
    SelectionArchive<Cs...> make_full_snapshot(entt::registry& reg)
    {
        SelectionArchive<Cs...> archive;
        archive.sel = nullptr; // No filter - capture all

        entt::snapshot snap{ reg };
        snap.get<entt::entity>(archive);
        (snap.template get<Cs>(archive), ...);

        return archive;
    }

} // namespace Core

