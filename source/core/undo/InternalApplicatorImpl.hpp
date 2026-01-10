#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "../reflection/Reflection.hpp"
#include <stdexcept>

// ===========================================================================
// Template Implementation for InternalApplicator
// ===========================================================================
// NOTE: This file does NOT include Types.hpp or any user-defined types.
// It is completely generic and uses only EnTT's reflection system.
// ===========================================================================

namespace Core {

    template<typename ValueTypes>
    entt::meta_any InternalApplicator<ValueTypes>::resolve(entt::entity e, entt::id_type compId) const {
        // Generic path using EnTT's storage system
        auto* storage = _reg->storage(compId);
        if (!storage) [[unlikely]] {
            return entt::meta_any{};
        }

        if (!storage->contains(e)) [[unlikely]] {
            return entt::meta_any{};
        }

        void* component_ptr = const_cast<void*>(storage->value(e));

        // Get the type_info from the storage - this is the actual C++ type
        auto type_info = storage->info();

        // Resolve the meta type using the type_info
        auto meta_type = entt::resolve(type_info);
        if (!meta_type) [[unlikely]] {
            return entt::meta_any{};
        }

        return meta_type.from_void(component_ptr);
    }

    template<typename ValueTypes>
    bool InternalApplicator<ValueTypes>::apply_impl(entt::entity e, entt::id_type compId, const reflection::Path& pathIds, const ValueTypes& value) {
        auto inst = resolve(e, compId);
        if (!inst) [[unlikely]] {
            return false;
        }
        SetByPath<ValueTypes>(inst, pathIds, value);
        return true;
    }

    template<typename ValueTypes>
    void InternalApplicator<ValueTypes>::SetField(const PatchType& p) {
        if (!apply_impl(p.entity, p.componentType, p.path, p.newValue)) [[unlikely]] {
            throw std::runtime_error("Component instance not found");
        }
    }

    template<typename ValueTypes>
    void InternalApplicator<ValueTypes>::Revert(const PatchType& p) {
        if (!apply_impl(p.entity, p.componentType, p.path, p.oldValue)) [[unlikely]] {
            throw std::runtime_error("Component instance not found");
        }
    }

}