#include "pch.h"
#include "ReflectTypes.h"

#include "../vendor/include/entt/entt.hpp"
#include "../vendor/include/entt/meta/meta.hpp"
#include "Types.h"
#include "core/UUID.h"
#include <app/sceneLayer/Scene.h>
#include "app/sceneLayer/components/Components.h"

// ----- Meta registration (call once before usage)
void ReflectTypes() {
    using namespace entt::literals;

    // Reflect Core::UUID
    entt::meta_factory<Core::UUID>()
        .type("UUID"_hs)  // Register type name
        .data<&Core::UUID::value, entt::as_ref_t>("value"_hs);

    // Reflect vec3 fields with as_ref policy
    entt::meta_factory<vec3>()
        .type("vec3"_hs)  // Register type name
        .data<&vec3::x, entt::as_ref_t>("x"_hs)
        .data<&vec3::y, entt::as_ref_t>("y"_hs)
        .data<&vec3::z, entt::as_ref_t>("z"_hs);

    // Reflect Transform fields with as_ref policy
    entt::meta_factory<Transform>()
        .type("Transform"_hs)  // Register type name - THIS IS CRITICAL!
        .data<&Transform::Position, entt::as_ref_t>("Position"_hs)
        .data<&Transform::Rotation, entt::as_ref_t>("Rotation"_hs)
        .data<&Transform::Scale, entt::as_ref_t>("Scale"_hs);

    entt::meta_factory<Parent>()
        .type("Parent"_hs)  // Register type name
        .data<&Parent::parentUUID, entt::as_ref_t>("parentUUID"_hs);

    entt::meta_factory<SceneData>()
        .type("Scene"_hs)  // Register type name - THIS IS CRITICAL!
        .data<&SceneData::_name, entt::as_ref_t>("name"_hs)
        .data<&SceneData::sceneColor, entt::as_ref_t>("sceneColor"_hs);

    // Reflect NameComponent
    entt::meta_factory<NameComponent>()
        .type("NameComponent"_hs)  // Register type name
        .data<&NameComponent::name, entt::as_ref_t>("name"_hs);


}
