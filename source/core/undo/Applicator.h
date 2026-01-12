#pragma once
#include "UndoManager.h"

namespace Core {

    // C++20 compile-time string wrapper for non-type template parameters
    template<std::size_t N>
    struct CompileTimeString {
        constexpr CompileTimeString(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        
        char value[N];
        static constexpr std::size_t size = N;
    };

    template<typename ValueTypes>
    class Applicator {
    public:
        explicit Applicator(UndoManager<ValueTypes>& undoManager) : _undoManager(undoManager) {}

        // SetField with compile-time string literal (C++20)
        // Usage: applicator.SetField<"Transform.Position.x">(entity, value);
        template<CompileTimeString PathStr, typename T>
        void SetField(entt::entity e, T&& newVal) {
            constexpr auto fullPath = PATH_FULL_CONSTEXPR(PathStr.value);
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            _undoManager.Execute(e, actualComponentTypeId, fullPath.propertyPath, ValueTypes{ std::forward<T>(newVal) });
        }

        // SetField with runtime string (for variables)
        template<typename T>
        void SetField(entt::entity e, const std::string& path_str, T&& newVal) {
            auto fullPath = reflection::ParseFullPathRuntime(path_str.c_str());
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