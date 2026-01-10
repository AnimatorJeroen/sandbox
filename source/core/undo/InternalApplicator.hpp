#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "Patch.hpp"

// ===========================================================================
// InternalApplicator: Applies undo/redo changes (templated on ValueTypes)
// ===========================================================================

namespace Core {


    template<typename ValueTypes>
    struct InternalApplicator {
        using PatchType = Patch<ValueTypes>;

        entt::registry* _reg;

        entt::meta_any resolve(entt::entity e, entt::id_type compId) const;
        bool apply_impl(entt::entity e, entt::id_type compId, const reflection::Path& pathIds, const ValueTypes& value);
        void SetField(const PatchType& p);
        void Revert(const PatchType& p);
        void SetContext(entt::registry& registry) {
            _reg = &registry;
        }
    };

}
// ===========================================================================
// Template Implementation
// ===========================================================================

#include "InternalApplicatorImpl.hpp"
