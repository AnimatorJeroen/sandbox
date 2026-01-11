#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "Patch.hpp"
#include "InternalApplicator.hpp"
#include "core/reflection/reflectionPathParser.hpp"
#include "core/reflection/Reflection.hpp"
#include <stack>
#include <vector>
#include <type_traits>
#include <concepts>
#include <optional>

// ===========================================================================
// UndoManager: Manages undo/redo stacks (templated directly on ValueTypes)
// ===========================================================================

namespace Core {


    template<typename ValueTypes>
    class UndoManager {
    public:
        using Patch = PatchT<ValueTypes>;
        using PatchGroup = PatchGroupT<ValueTypes>;
        using InternalApplicator = InternalApplicatorT<ValueTypes>;

        explicit UndoManager();


        void SetContext(entt::registry& registry) {
            _internalApplicator.SetContext(registry);
        }


        // Execute a change with FullPath (component type extracted from path string)
        template<typename T>
        void Execute(entt::entity e, const reflection::FullPath& fullPath, T&& value) {
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            Execute(e, actualComponentTypeId, fullPath.propertyPath, ValueTypes{ std::forward<T>(value) });
        }

        // Execute a change with Path (sentinel-terminated fixed-size array)
        void Execute(entt::entity e,
            entt::id_type compId,
            const reflection::Path& pathIds,
            const ValueTypes& newVal);

        // Begin recording patches for bundling
        void BeginUndo();

        // End recording and push all bundled patches as a single undo step
        void EndUndo();

        // Undo the last change
        bool Undo();

        // Redo the last undone change
        bool Redo();

        // Check if undo is available
        [[nodiscard]] bool CanUndo() const noexcept;

        // Check if redo is available
        [[nodiscard]] bool CanRedo() const noexcept;

        // Get the size of the undo stack
        [[nodiscard]] size_t UndoStackSize() const noexcept;

        // Get the size of the redo stack
        [[nodiscard]] size_t RedoStackSize() const noexcept;

        // Clear all history
        void Clear();

        // Check if currently recording a bundle
        [[nodiscard]] bool IsRecording() const noexcept;

    private:
        // Build a patch (capture old via meta)
        Patch make_patch(entt::entity e,
            entt::id_type compId,
            const reflection::Path& pathIds,
            const ValueTypes& newVal);

        // Helper: Resolve component name hash to actual component type ID
        static entt::id_type resolve_component_type(entt::id_type nameHash) {
            auto meta_type = entt::resolve(nameHash);
            if (!meta_type) {
                throw std::runtime_error("Component type not found in reflection system");
            }
            return meta_type.info().hash();
        }

        InternalApplicator _internalApplicator;
        std::stack<PatchGroup> _undo_stack;
        std::stack<PatchGroup> _redo_stack;

        // Recording state for bundling patches
        bool _recording = false;
        std::optional<PatchGroup> _current_group;
    };

}
// ===========================================================================
// Template Implementation
// ===========================================================================

#include "UndoManagerImpl.hpp"
