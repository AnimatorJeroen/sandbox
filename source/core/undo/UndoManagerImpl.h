#pragma once

#include "../reflection/Reflection.h"
#include <iostream>
#include <stdexcept>
#include <memory>
#include "ops/SetFieldOp.h"

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
	void UndoManager<ValueTypes>::SetField(
        entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        const ValueTypes& newVal) {
        // Create SetFieldOp (captures old value)
        auto op = std::make_unique<Core::SetFieldOp<ValueTypes>>(*_registry, e, compId, pathIds, newVal);

        // Apply the change
        op->Apply();

        // If we're recording, add to current command
        if (_recording && _current_command.has_value()) {
            _current_command->AddOp(std::move(op));
        }
        else {
            // Otherwise, push as a single-op command to undo stack
            UndoableCommand command;
            command.AddOp(std::move(op));
            _undo_stack.push(std::move(command));

            // Clear redo stack when a new action is performed
            while (!_redo_stack.empty()) {
                _redo_stack.pop();
            }
        }
    }

    // Begin recording operations for bundling
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::BeginUndo() {
        if (_recording) {
            throw std::runtime_error("BeginUndo() called while already recording. Call EndUndo() first.");
        }
        _recording = true;
        _current_command = UndoableCommand();
    }

    // End recording and push all bundled operations as a single undo step
    template<typename ValueTypes>
    void UndoManager<ValueTypes>::EndUndo() {
        if (!_recording) {
            throw std::runtime_error("EndUndo() called without matching BeginUndo()");
        }

        _recording = false;

        // Only push to undo stack if we have operations
        if (_current_command.has_value() && !_current_command->Empty()) {
            _undo_stack.push(std::move(*_current_command));
            _current_command.reset();

            // Clear redo stack when a new action is performed
            while (!_redo_stack.empty()) {
                _redo_stack.pop();
            }
        }
        else {
            _current_command.reset();
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

        UndoableCommand command = std::move(_undo_stack.top());
        _undo_stack.pop();

        // Revert the command (operations are reverted in reverse order internally)
        command.Revert();

        // Move to redo stack
        _redo_stack.push(std::move(command));

        return true;
    }

    // Redo the last undone change
    template<typename ValueTypes>
    bool UndoManager<ValueTypes>::Redo() {
        if (_redo_stack.empty()) {
            return false;
        }

        UndoableCommand command = std::move(_redo_stack.top());
        _redo_stack.pop();

        // Apply the command (operations are applied in forward order)
        command.Apply();

        // Move back to undo stack
        _undo_stack.push(std::move(command));

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
        _current_command.reset();
    }

}