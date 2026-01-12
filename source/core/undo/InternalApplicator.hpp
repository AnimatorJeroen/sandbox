#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "Patch.hpp"

// ===========================================================================
// InternalApplicatorT: Applies undo/redo changes (templated on ValueTypes)
// ===========================================================================

namespace Core {

    template<typename ValueTypes>
    struct InternalApplicatorT {
        using Patch = PatchT<ValueTypes>;

        entt::registry* _reg;

        entt::meta_any resolve(entt::entity e, entt::id_type compId) const;
        bool SetField(entt::entity e, entt::id_type compId, const reflection::Path& pathIds, const ValueTypes& value);
        void Apply(const Patch& p);
        void Revert(const Patch& p);
        void SetContext(entt::registry& registry) {
            _reg = &registry;
        }
    };

}
// ===========================================================================
// Template Implementation
// ===========================================================================

#include "InternalApplicatorImpl.hpp"
