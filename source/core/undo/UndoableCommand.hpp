#pragma once

#include "ops/IOp.hpp"
#include <vector>
#include <memory>

// ===========================================================================
// UndoableCommand: Container for multiple operations (replaces PatchGroup)
// ===========================================================================

namespace Core {

    class UndoableCommand {
    public:
        // Default constructor
        UndoableCommand() = default;

        // Construct with initial capacity
        explicit UndoableCommand(size_t reserveSize) {
            _operations.reserve(reserveSize);
        }

        // Add an operation to the command
        void AddOp(std::unique_ptr<IOp> op) {
            _operations.push_back(std::move(op));
        }

        // Apply all operations in forward order
        void Apply() {
            for (auto& op : _operations) {
                op->Apply();
            }
        }

        // Revert all operations in reverse order
        void Revert() {
            for (auto it = _operations.rbegin(); it != _operations.rend(); ++it) {
                (*it)->Revert();
            }
        }

        // Check if command is empty
        [[nodiscard]] bool Empty() const noexcept {
            return _operations.empty();
        }

        // Get number of operations in command
        [[nodiscard]] size_t Size() const noexcept {
            return _operations.size();
        }

    private:
        std::vector<std::unique_ptr<IOp>> _operations;
    };

}
