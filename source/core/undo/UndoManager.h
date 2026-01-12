#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "UndoableCommand.h"
#include "core/reflection/Reflection.h"
#include <stack>
#include <vector>
#include <optional>

// ===========================================================================
// UndoManager: Manages undo/redo stacks (templated directly on FieldTypes)
// ===========================================================================

namespace Core {


    template<typename FieldTypes>
    class UndoManager {
    public:
        explicit UndoManager();
        void SetContext(entt::registry& registry) {
            _registry = &registry;
        }


        // Execute a change with FullPath (component type extracted from path string)
        template<typename T>
        void SetField(entt::entity e, const reflection::FullPath& fullPath, T&& value) {
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            Execute(e, actualComponentTypeId, fullPath.propertyPath, FieldTypes{ std::forward<T>(value) });
        }

        // Execute a change with Path (sentinel-terminated fixed-size array)
        void SetField(entt::entity e,
            entt::id_type compId,
            const reflection::Path& pathIds,
            const FieldTypes& newVal);

        // Create entities from selection snapshot
        template<typename... Cs>
        void Create(const std::unordered_set<entt::entity>& selection);

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
        // Helper: Resolve component name hash to actual component type ID
        static entt::id_type resolve_component_type(entt::id_type nameHash) {
            auto meta_type = entt::resolve(nameHash);
            if (!meta_type) {
                throw std::runtime_error("Component type not found in reflection system");
            }
            return meta_type.info().hash();
        }

        entt::registry* _registry;
        std::stack<UndoableCommand> _undo_stack;
        std::stack<UndoableCommand> _redo_stack;

        // Recording state for bundling patches
        bool _recording = false;
        std::optional<UndoableCommand> _current_command;

    };

}
// ===========================================================================
// Template Implementation
// ===========================================================================

#include "UndoManagerImpl.h"
