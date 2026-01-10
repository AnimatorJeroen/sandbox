#pragma once
#include "UndoManager.hpp"

namespace Core {

    template<typename ValueTypes>
    class Applicator {
    public:
        explicit Applicator(UndoManager<ValueTypes>& undoManager) : _undoManager(undoManager) {}

        // Apply with compile-time parsed path (PATH_FULL_CONSTEXPR)
        template<typename T>
        void Apply(entt::entity e, const reflection::FullPath& fullPath, T&& newVal) {
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            _undoManager.Execute(e, actualComponentTypeId, fullPath.propertyPath, ValueTypes{ std::forward<T>(newVal) });
        }

        // Begin recording patches for bundling into a single undo step
        void BeginUndo() {
            _undoManager.BeginUndo();
        }

        // End recording and push all bundled patches as a single undo step
        void EndUndo() {
            _undoManager.EndUndo();
        }

        // Check if currently recording a bundle
        [[nodiscard]] bool IsRecording() const noexcept {
            return _undoManager.IsRecording();
        }

    private:
        UndoManager<ValueTypes>& _undoManager;

        // Helper: Resolve component name hash to actual component type ID
        static entt::id_type resolve_component_type(entt::id_type nameHash) {
            auto meta_type = entt::resolve(nameHash);
            if (!meta_type) {
                throw std::runtime_error("Component type not found in reflection system");
            }
            return meta_type.info().hash();
        }
    };

}