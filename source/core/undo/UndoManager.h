#pragma once
#include <vector>
#include <memory>
#include "Undo.h"

namespace Core {

class UndoManager {
public:
    using Command = UndoableCommand;

    void Push(const Command& cmd) {
        _undoStack.push_back(cmd);
        _redoStack.clear();
    }

    bool CanUndo() const { return !_undoStack.empty(); }
    bool CanRedo() const { return !_redoStack.empty(); }

    template<class Registry>
    bool Undo(Registry& registry) {
        if (_undoStack.empty()) return false;
        const auto cmd = _undoStack.back();
        _undoStack.pop_back();
        cmd.Revert(registry);
        _redoStack.push_back(cmd);
        return true;
    }

    template<class Registry>
    bool Redo(Registry& registry) {
        if (_redoStack.empty()) return false;
        const auto cmd = _redoStack.back();
        _redoStack.pop_back();
        cmd.Apply(registry);
        _undoStack.push_back(cmd);
        return true;
    }

    // Non-templated no-op versions for wiring without a registry
    bool Undo() { if (_undoStack.empty()) return false; _redoStack.push_back(_undoStack.back()); _undoStack.pop_back(); return true; }
    bool Redo() { if (_redoStack.empty()) return false; _undoStack.push_back(_redoStack.back()); _redoStack.pop_back(); return true; }

private:
    std::vector<Command> _undoStack;
    std::vector<Command> _redoStack;
};

} // namespace Core
