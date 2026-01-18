#pragma once

// ===========================================================================
// IOp: Base interface for undo/redo operations
// ===========================================================================

namespace Core {

    class IOp {
    public:
        virtual ~IOp() = default;

        // Apply the operation (execute forward)
        virtual void Apply() = 0;

        // Revert the operation (undo)
        virtual void Revert() = 0;
    };

}
