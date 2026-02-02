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
        : _ctx(nullptr) {
    }

    // Execute a change and push it onto the undo stack
    template<typename FieldTypes>
	void UndoManager<FieldTypes>::SetField(
        entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        const FieldTypes& newVal) {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        // Create SetFieldOp (captures old value)
        auto op = std::make_unique<Core::SetFieldOp<FieldTypes>>(*_ctx->registry, e, compId, pathIds, newVal);

        // Apply the change
        op->Apply();

        // If we're recording, add to current command
        if (_ctx->recording && _ctx->currentCommand.has_value()) {
            _ctx->currentCommand->AddOp(std::move(op));
        }
        else {
            // Otherwise, push as a single-op command to undo stack
            UndoableCommand command;
            command.AddOp(std::move(op));
            _ctx->undoStack.push(std::move(command));

            // Clear redo stack when a new action is performed
            while (!_ctx->redoStack.empty()) {
                _ctx->redoStack.pop();
            }
        }
    }

    template<typename FieldTypes>
    template<typename... Cs>
    void UndoManager<FieldTypes>::CaptureCreate(const std::unordered_set<entt::entity>& selection)
    {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        auto op = std::make_unique<Core::CaptureCreateOp<Cs...>>(
            *_ctx->registry, selection);

        _ctx->registry->remove<Cs...>(selection.begin(), selection.end());

        // Apply the change
        op->Apply();

        // If we're recording, add to current command
        if (_ctx->recording && _ctx->currentCommand.has_value()) {
            _ctx->currentCommand->AddOp(std::move(op));
        }
        else {
            // Otherwise, push as a single-op command to undo stack
            UndoableCommand command;
            command.AddOp(std::move(op));
            _ctx->undoStack.push(std::move(command));

            // Clear redo stack when a new action is performed
            while (!_ctx->redoStack.empty()) {
                _ctx->redoStack.pop();
            }
        }

	}

    template<typename FieldTypes>
    template<typename... Cs>
    void UndoManager<FieldTypes>::CaptureDelete(const std::unordered_set<entt::entity>& selection)
    {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        auto op = std::make_unique<Core::CaptureDeleteOp<Cs...>>(
            *_ctx->registry, selection);

        // Apply the change
        op->Apply();

        // If we're recording, add to current command
        if (_ctx->recording && _ctx->currentCommand.has_value()) {
            _ctx->currentCommand->AddOp(std::move(op));
        }
        else {
            // Otherwise, push as a single-op command to undo stack
            UndoableCommand command;
            command.AddOp(std::move(op));
            _ctx->undoStack.push(std::move(command));

            // Clear redo stack when a new action is performed
            while (!_ctx->redoStack.empty()) {
                _ctx->redoStack.pop();
            }
        }

    }

    template<typename FieldTypes>
    template<typename... Cs>
    void UndoManager<FieldTypes>::CaptureComponentChange(
        const std::unordered_set<entt::entity>& entities,
        SelectionArchive<Cs...>&& beforeState,
        SelectionArchive<Cs...>&& afterState)
    {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        auto op = std::make_unique<Core::CaptureComponentChangeOp<Cs...>>(
            *_ctx->registry, entities, std::move(beforeState), std::move(afterState));

        // Apply the change (sets to "after" state)
        op->Apply();

        // If we're recording, add to current command
        if (_ctx->recording && _ctx->currentCommand.has_value()) {
            _ctx->currentCommand->AddOp(std::move(op));
        }
        else {
            // Otherwise, push as a single-op command to undo stack
            UndoableCommand command;
            command.AddOp(std::move(op));
            _ctx->undoStack.push(std::move(command));

            // Clear redo stack when a new action is performed
            while (!_ctx->redoStack.empty()) {
                _ctx->redoStack.pop();
            }
        }
    }

    // Begin recording operations for bundling
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::BeginUndo() {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        if (_ctx->recording) {
            throw std::runtime_error("BeginUndo() called while already recording. Call EndUndo() first.");
        }
        _ctx->recording = true;
        _ctx->currentCommand = UndoableCommand();
    }

    // End recording and push all bundled operations as a single undo step
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::EndUndo() {
        if (!_ctx) {
            throw std::runtime_error("No active context. Call SetContext() first.");
        }
        
        if (!_ctx->recording) {
            throw std::runtime_error("EndUndo() called without matching BeginUndo()");
        }

        _ctx->recording = false;

        // Only push to undo stack if we have operations
        if (_ctx->currentCommand.has_value() && !_ctx->currentCommand->Empty()) {
            _ctx->undoStack.push(std::move(*_ctx->currentCommand));
            _ctx->currentCommand.reset();
            _ctx->isDirty = true;

            // Clear redo stack when a new action is performed
            while (!_ctx->redoStack.empty()) {
                _ctx->redoStack.pop();
            }
        }
        else {
            _ctx->currentCommand.reset();
        }
    }

    // Check if currently recording a bundle
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::IsRecording() const noexcept {
        return _ctx ? _ctx->recording : false;
    }

    // Undo the last change
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::Undo() {
        if (!_ctx || _ctx->undoStack.empty()) {
            return false;
        }

        UndoableCommand command = std::move(_ctx->undoStack.top());
        _ctx->undoStack.pop();

        // Revert the command (operations are reverted in reverse order internally)
        command.Revert();
        _ctx->isDirty = true;

        // Move to redo stack
        _ctx->redoStack.push(std::move(command));

        return true;
    }

    // Redo the last undone change
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::Redo() {
        if (!_ctx || _ctx->redoStack.empty()) {
            return false;
        }

        UndoableCommand command = std::move(_ctx->redoStack.top());
        _ctx->redoStack.pop();

        // Apply the command (operations are applied in forward order)
        command.Apply();
        _ctx->isDirty = true;

        // Move back to undo stack
        _ctx->undoStack.push(std::move(command));

        return true;
    }

    // Check if undo is available
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::CanUndo() const noexcept {
        return _ctx && !_ctx->undoStack.empty();
    }

    // Check if redo is available
    template<typename FieldTypes>
    bool UndoManager<FieldTypes>::CanRedo() const noexcept {
        return _ctx && !_ctx->redoStack.empty();
    }

    // Get the size of the undo stack
    template<typename FieldTypes>
    size_t UndoManager<FieldTypes>::UndoStackSize() const noexcept {
        return _ctx ? _ctx->undoStack.size() : 0;
    }

    // Get the size of the redo stack
    template<typename FieldTypes>
    size_t UndoManager<FieldTypes>::RedoStackSize() const noexcept {
        return _ctx ? _ctx->redoStack.size() : 0;
    }

    // Clear all history
    template<typename FieldTypes>
    void UndoManager<FieldTypes>::Clear() {
        if (!_ctx) {
            return;
        }
        
        while (!_ctx->undoStack.empty()) _ctx->undoStack.pop();
        while (!_ctx->redoStack.empty()) _ctx->redoStack.pop();
        _ctx->recording = false;
        _ctx->currentCommand.reset();
    }

}