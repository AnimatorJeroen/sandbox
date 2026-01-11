#pragma once

#include "../reflection/Reflection.hpp"
#include <iostream>
#include <stdexcept>

// ===========================================================================
// Template Implementation for UndoManager
// ===========================================================================

namespace Core {


    template<typename ValueTypes>
    UndoManager<ValueTypes>::UndoManager()
        : _recording(false) {
    }

    // Execute a change and push it onto the undo stack
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::Execute(entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        const ValueTypes& newVal) {
        // Create patch (captures old value)
        Patch patch = make_patch(e, compId, pathIds, newVal);

        // SetField the change
        _internalApplicator.SetField(patch);

        // If we're recording, add to current group
        if (_recording && _current_group.has_value()) {
            _current_group->Add(std::move(patch));
        }
        else {
            // Otherwise, push as a single-patch group to undo stack
            PatchGroup group;
            group.Add(std::move(patch));
            _undo_stack.push(std::move(group));

            // Clear redo stack when a new action is performed
            while (!_redo_stack.empty()) {
                _redo_stack.pop();
            }
        }
    }

    // Begin recording patches for bundling
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::BeginUndo() {
        if (_recording) {
            throw std::runtime_error("BeginUndo() called while already recording. Call EndUndo() first.");
        }
        _recording = true;
        _current_group = PatchGroup();
    }

    // End recording and push all bundled patches as a single undo step
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::EndUndo() {
        if (!_recording) {
            throw std::runtime_error("EndUndo() called without matching BeginUndo()");
        }

        _recording = false;

        // Only push to undo stack if we have patches
        if (_current_group.has_value() && !_current_group->Empty()) {
            _undo_stack.push(std::move(*_current_group));
            _current_group.reset();

            // Clear redo stack when a new action is performed
            while (!_redo_stack.empty()) {
                _redo_stack.pop();
            }
        }
        else {
            _current_group.reset();
        }
    }

    // Check if currently recording a bundle
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::IsRecording() const noexcept {
        return _recording;
    }

    // Undo the last change
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::Undo() {
        if (_undo_stack.empty()) {
            return false;
        }

        PatchGroup group = std::move(_undo_stack.top());
        _undo_stack.pop();

        // Revert all patches in the group in reverse order
        for (auto it = group.patches.rbegin(); it != group.patches.rend(); ++it) {
            _internalApplicator.Revert(*it);
        }

        // Move to redo stack
        _redo_stack.push(std::move(group));

        return true;
    }

    // Redo the last undone change
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::Redo() {
        if (_redo_stack.empty()) {
            return false;
        }

        PatchGroup group = std::move(_redo_stack.top());
        _redo_stack.pop();

        // SetField all patches in the group in forward order
        for (auto& patch : group.patches) {
            _internalApplicator.SetField(patch);
        }

        // Move back to undo stack
        _undo_stack.push(std::move(group));

        return true;
    }

    // Check if undo is available
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::CanUndo() const noexcept {
        return !_undo_stack.empty();
    }

    // Check if redo is available
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::CanRedo() const noexcept {
        return !_redo_stack.empty();
    }

    // Get the size of the undo stack
    template<typename ValueTypes>
    size_t UndoManager<ValueTypes>::UndoStackSize() const noexcept {
        return _undo_stack.size();
    }

    // Get the size of the redo stack
    template<typename ValueTypes>
    size_t UndoManager<ValueTypes>::RedoStackSize() const noexcept {
        return _redo_stack.size();
    }

    // Clear all history
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::Clear() {
        while (!_undo_stack.empty()) _undo_stack.pop();
        while (!_redo_stack.empty()) _redo_stack.pop();
        _recording = false;
        _current_group.reset();
    }

    // Build a patch (capture old via meta)
    template<typename ValueTypes>
    typename UndoManager<ValueTypes>::Patch UndoManager<ValueTypes>::make_patch(entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        const ValueTypes& newVal) {
        auto inst = _internalApplicator.resolve(e, compId);
        if (!inst) throw std::runtime_error("Component instance not found for make_patch");
        ValueTypes oldVal = GetByPath<ValueTypes>(inst, pathIds);
        return Patch{ e, compId, pathIds, oldVal, newVal };
    }
}