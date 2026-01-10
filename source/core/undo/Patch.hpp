#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "../reflection/Reflection.hpp"
#include "../reflection/reflectionPathParser.hpp"
#include <iostream>
#include <vector>

// ===========================================================================
// Patch: Stores undo/redo information (templated on ValueTypes)
// ===========================================================================
// Uses a sentinel-terminated fixed-size array (max 16 elements)
// ===========================================================================

namespace Core {


    template<typename ValueTypes_>
    struct Patch {
        using ValueTypes = ValueTypes_;  // Expose ValueTypes for macro usage

        entt::entity entity{};
        entt::id_type componentType{};   // e.g., entt::type_id<Transform>().hash()
        reflection::Path path{};          // Sentinel-terminated fixed-size array
        ValueTypes oldValue{}, newValue{};

        // Construct from Path
        Patch(entt::entity e, entt::id_type comp, const reflection::Path& p, ValueTypes old_val, ValueTypes new_val)
            : entity(e), componentType(comp), path(p),
            oldValue(std::move(old_val)), newValue(std::move(new_val)) {
        }

        // Default constructor
        Patch() = default;

        // Helpers to print patch info (optional)
        void Dump() const {
            std::cout << "Patch: comp=" << componentType
                << " pathLength=" << reflection::PathLength(path)
                << " (old/new captured)\n";
        }
    };

    // ===========================================================================
    // PatchGroup: Bundles multiple patches as a single undo/redo step
    // ===========================================================================

    template<typename ValueTypes_>
    struct PatchGroup {
        using ValueTypes = ValueTypes_;
        using PatchType = Patch<ValueTypes>;

        std::vector<PatchType> patches;

        // Default constructor
        PatchGroup() = default;

        // Construct with initial capacity
        explicit PatchGroup(size_t reserveSize) {
            patches.reserve(reserveSize);
        }

        // Add a patch to the group
        void Add(PatchType&& patch) {
            patches.push_back(std::move(patch));
        }

        // Check if group is empty
        [[nodiscard]] bool Empty() const noexcept {
            return patches.empty();
        }

        // Get number of patches in group
        [[nodiscard]] size_t Size() const noexcept {
            return patches.size();
        }
    };

}