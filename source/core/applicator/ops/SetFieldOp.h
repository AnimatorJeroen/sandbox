#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "IOp.h"
#include "core/reflection/Reflection.h"
#include "core/UUID.h"

// ===========================================================================
// SetFieldOp: Operation that sets a field value (templated on FieldTypes)
// ===========================================================================

namespace Core {

    template<typename FieldTypes>
    class SetFieldOp : public IOp {
    public:
        SetFieldOp(Core::Registry& reg,
            entt::entity e,
            entt::id_type compId,
            const reflection::Path& pathIds,
            FieldTypes newVal);

        void Apply() override;
        void Revert() override;

    private:

        entt::meta_any resolve() const;
        bool setField(const FieldTypes& value) const;

        Core::Registry& _reg;
        UUID _entityId;
        entt::id_type _componentType;
        reflection::Path _pathIds;
        FieldTypes _oldValue;
        FieldTypes _newValue;
    };



    // ===========================================================================
    // Template Implementation
    // ===========================================================================

    template<typename FieldTypes>
    SetFieldOp<FieldTypes>::SetFieldOp(
        Core::Registry& reg,
        entt::entity e,
        entt::id_type compId,
        const reflection::Path& pathIds,
        FieldTypes newVal)
        : _reg(reg)
        , _componentType(compId)
        , _pathIds(pathIds)
        , _newValue(std::move(newVal))
    {
		_entityId = _reg.get<Core::UUID>(e);
        auto inst = resolve();
        if (!inst) throw std::runtime_error("Component instance not found for make_set_field_op");
        _oldValue = GetByPath<FieldTypes>(inst, pathIds);
    }

    template<typename FieldTypes>
    void SetFieldOp<FieldTypes>::Apply() {
        if (!setField(_newValue)) {
            throw std::runtime_error("SetFieldOp::Apply failed - component instance not found");
        }
    }

    template<typename FieldTypes>
    void SetFieldOp<FieldTypes>::Revert() {
        if (!setField(_oldValue)) {
            throw std::runtime_error("SetFieldOp::Revert failed - component instance not found");
        }
    }

    template<typename FieldTypes>
    entt::meta_any SetFieldOp<FieldTypes>::resolve() const {
        // Generic path using EnTT's storage system
        auto* storage = _reg.storage(_componentType);
        if (!storage) {
            return entt::meta_any{};
        }

		entt::entity entity = entt::null;
        for (auto e : _reg.view<UUID>()) {
            if (_reg.get<UUID>(e).value == _entityId.value) {
				entity = e;
                break;
            }
        }

        if (!storage->contains(entity)) {
            return entt::meta_any{};
        }

        void* component_ptr = const_cast<void*>(storage->value(entity));

        // Get the type_info from the storage - this is the actual C++ type
        auto type_info = storage->info();

        // Resolve the meta type using the type_info
        auto meta_type = entt::resolve(type_info);
        if (!meta_type) {
            return entt::meta_any{};
        }

        return meta_type.from_void(component_ptr);
    }

    template<typename FieldTypes>
    bool SetFieldOp<FieldTypes>::setField(const FieldTypes& value) const {
        auto inst = resolve();
        if (!inst) {
            return false;
        }
        SetByPath<FieldTypes>(inst, _pathIds, value);
        return true;
    }
}