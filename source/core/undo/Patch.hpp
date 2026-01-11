#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "../reflection/Reflection.hpp"
#include "../reflection/reflectionPathParser.hpp"
#include <iostream>
#include <vector>

// ===========================================================================
// PatchT: Stores undo/redo information (templated on ValueTypes)
// ===========================================================================
// Uses a sentinel-terminated fixed-size array (max 16 elements)
// ===========================================================================

namespace Core {


    template<typename ValueTypes_>
    struct PatchT {
        using ValueTypes = ValueTypes_;  // Expose ValueTypes for macro usage

        entt::entity entity{};
        entt::id_type componentType{};   // e.g., entt::type_id<Transform>().hash()
        reflection::Path path{};          // Sentinel-terminated fixed-size array
        ValueTypes oldValue{}, newValue{};

        // Construct from Path
        PatchT(entt::entity e, entt::id_type comp, const reflection::Path& p, ValueTypes old_val, ValueTypes new_val)
            : entity(e), componentType(comp), path(p),
            oldValue(std::move(old_val)), newValue(std::move(new_val)) {
        }

        // Default constructor
        PatchT() = default;

        // Helpers to print patch info (optional)
        void Dump() const {
            std::cout << "Patch: comp=" << componentType
                << " pathLength=" << reflection::PathLength(path)
                << " (old/new captured)\n";
        }
    };

    // ===========================================================================
    // PatchGroupT: Bundles multiple patches as a single undo/redo step
    // ===========================================================================

    template<typename ValueTypes_>
    struct PatchGroupT {
        using ValueTypes = ValueTypes_;
        using Patch = PatchT<ValueTypes>;

        std::vector<Patch> patches;

        // Default constructor
        PatchGroupT() = default;

        // Construct with initial capacity
        explicit PatchGroupT(size_t reserveSize) {
            patches.reserve(reserveSize);
        }

        // Add a patch to the group
        void Add(Patch&& patch) {
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