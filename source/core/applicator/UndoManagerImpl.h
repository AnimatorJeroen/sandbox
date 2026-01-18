#pragma once

#include "../reflection/Reflection.h"
#include <iostream>
#include <stdexcept>
#include <memory>
#include "ops/SetFieldOp.h"
#include "ops/StructuralOps.h"
#include "UndoManager.h"

// ===========================================================================
// Template Implementation for UndoManager
// ===========================================================================

namespace Core {

    template<typename FieldTypes>
    UndoManager<FieldTypes>::UndoManager()
        : _recording(false) {
    }

    // Execute a change and push it onto the undo stack
    template<typename FieldTypes>
	void UndoManager<FieldTypes>::SetField(
        entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        const FieldTypes& newVal) {
        // Create SetFieldOp (captures old value)
        auto op = std::make_unique<Core::SetFieldOp<FieldTypes>>(*_registry, e, compId, pathIds, newVal);

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

    template<typename FieldTypes>
    template<typename... Cs>
    void UndoManager<FieldTypes>::CaptureCreate(const std::unordered_set<entt::entity>& selection)
    {
        auto op = std::make_unique<Core::CaptureCreateOp<Cs...>>(
            *_registry, selection);

        _registry->remove<Cs...>(selection.begin(), selection.end());

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

    template<typename FieldTypes>
    template<typename... Cs>
    void UndoManager<FieldTypes>::CaptureDelete(const std::unordered_set<entt::entity>& selection)
    {
        auto op = std::make_unique<Core::CaptureDeleteOp<Cs...>>(
            *_registry, selection);

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
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::BeginUndo() {
        if (_recording) {
            throw std::runtime_error("BeginUndo() called while already recording. Call EndUndo() first.");
        }
        _recording = true;
        _current_command = UndoableCommand();
    }

    // End recording and push all bundled operations as a single undo step
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::EndUndo() {
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
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::IsRecording() const noexcept {
        return _recording;
    }

    // Undo the last change
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::Undo() {
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
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::Redo() {
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
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::CanUndo() const noexcept {
        return !_undo_stack.empty();
    }

    // Check if redo is available
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::CanRedo() const noexcept {
        return !_redo_stack.empty();
    }

    // Get the size of the undo stack
    template<typename FieldTypes>
    size_t UndoManager<FieldTypes>::UndoStackSize() const noexcept {
        return _undo_stack.size();
    }

    // Get the size of the redo stack
    template<typename FieldTypes>
    size_t UndoManager<FieldTypes>::RedoStackSize() const noexcept {
        return _redo_stack.size();
    }

    // Clear all history
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::Clear() {
        while (!_undo_stack.empty()) _undo_stack.pop();
        while (!_redo_stack.empty()) _redo_stack.pop();
        _recording = false;
        _current_command.reset();
    }

}