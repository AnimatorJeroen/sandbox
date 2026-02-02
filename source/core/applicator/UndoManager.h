#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "UndoableCommand.h"
#include "core/reflection/Reflection.h"
#include <stack>
#include <vector>
#include <optional>
#include <unordered_map>

// ===========================================================================
// UndoManager: Manages undo/redo stacks (templated directly on FieldTypes)
// ===========================================================================

namespace Core {

    // Forward declaration
    template<class... Cs>
    struct SelectionArchive;

    // Context struct that holds undo/redo state for a specific registry
    struct UndoContext {
        entt::registry* registry = nullptr;
        std::stack<UndoableCommand> undoStack;
        std::stack<UndoableCommand> redoStack;
        bool recording = false;
        std::optional<UndoableCommand> currentCommand;
		bool isDirty = false;
    };

    template<typename FieldTypes>
    class UndoManager {
    public:
        explicit UndoManager();
        
        void SetContext(entt::registry* registry) {
            auto it = _contexts.find(registry);
            if (it == _contexts.end()) {
                // Create new context for this registry
                UndoContext newContext;
                newContext.registry = registry;
                _contexts[registry] = std::move(newContext);
            }
            // Switch to this context (always valid after this call)
            _ctx = &_contexts[registry];
        }

        void EraseContext(entt::registry* registry)
        {
            auto it = _contexts.find(registry);
            if (it != _contexts.end()) {
                _contexts.erase(it);
            }
        }

        bool IsContextDirty(entt::registry& registry) const
        {
            auto it = _contexts.find(&registry);
            if (it != _contexts.end()) {
                return it->second.isDirty;
            }
            return false;
        }

        void MarkContextDirty(entt::registry& registry, bool dirty)
        {
            auto it = _contexts.find(&registry);
            if (it != _contexts.end()) {
                it->second.isDirty = dirty;
            }
        }

        // Execute a change with FullPath (component type extracted from path string)
        template<typename T>
        void SetField(entt::entity e, const reflection::FullPath& fullPath, T&& value) {
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            SetField(e, actualComponentTypeId, fullPath.propertyPath, FieldTypes{ std::forward<T>(value) });
        }

        // Execute a change with Path (sentinel-terminated fixed-size array)
        void SetField(entt::entity e,
            entt::id_type compId,
            const reflection::Path& pathIds,
            const FieldTypes& newVal);

        // Create entities from selection snapshot
        template<typename... Cs>
        void CaptureCreate(const std::unordered_set<entt::entity>& selection);

        // Delete entities from selection snapshot
        template<typename... Cs>
        void CaptureDelete(const std::unordered_set<entt::entity>& selection);

        // Capture component add/remove changes (takes before and after snapshots)
        template<typename... Cs>
        void CaptureComponentChange(
            const std::unordered_set<entt::entity>& entities,
            SelectionArchive<Cs...>&& beforeState,
            SelectionArchive<Cs...>&& afterState);

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

        // Get the registry pointer
        [[nodiscard]] entt::registry* GetRegistry() const noexcept {
            return _ctx->registry;
        }

    private:
        // Helper: Resolve component name hash to actual component type ID
        static entt::id_type resolve_component_type(entt::id_type nameHash) {
            auto meta_type = entt::resolve(nameHash);
            if (!meta_type) {
                throw std::runtime_error("Component type not found in reflection system");
            }
            return meta_type.info().hash();
        }

        // Map of registry pointers to their undo contexts
        std::unordered_map<entt::registry*, UndoContext> _contexts;
        
        // Pointer to the currently active context (always valid, points to _dummyContext or a context in _contexts)
        UndoContext* _ctx;
    };

}
// ===========================================================================
// Template Implementation
// ===========================================================================

#include "UndoManagerImpl.h"
