#include "ReflectTypes.hpp"

#include "../vendor/include/entt/entt.hpp"
#include "../vendor/include/entt/meta/meta.hpp"
#include "Types.hpp"
#include "core/UUID.h"
#include <app/sceneLayer/Scene.h>

// ----- Meta registration (call once before usage)
void ReflectTypes() {
    using namespace entt::literals;

    // Reflect Core::UUID
    entt::meta_factory<Core::UUID>()
        .type("UUID"_hs)  // Register type name
        .data<&Core::UUID::value, entt::as_ref_t>("value"_hs);

    // Reflect Vec3 fields with as_ref policy
    entt::meta_factory<Vec3>()
        .type("Vec3"_hs)  // Register type name
        .data<&Vec3::x, entt::as_ref_t>("x"_hs)
        .data<&Vec3::y, entt::as_ref_t>("y"_hs)
        .data<&Vec3::z, entt::as_ref_t>("z"_hs);

    // Reflect Matrix2x3 fields
    entt::meta_factory<Matrix2x3>()
        .type("Matrix2x3"_hs)  // Register type name
        .data<&Matrix2x3::data, entt::as_ref_t>("data"_hs);

    // Reflect Transform fields with as_ref policy
    entt::meta_factory<Transform>()
        .type("Transform"_hs)  // Register type name - THIS IS CRITICAL!
        .data<&Transform::Position, entt::as_ref_t>("Position"_hs)
        .data<&Transform::Scale, entt::as_ref_t>("Scale"_hs)
        .data<&Transform::Weights, entt::as_ref_t>("Weights"_hs)
        .data<&Transform::Matrix, entt::as_ref_t>("Matrix"_hs);

    entt::meta_factory<SceneData>()
        .type("Scene"_hs)  // Register type name - THIS IS CRITICAL!
        .data<&SceneData::_name, entt::as_ref_t>("name"_hs)
        .data<&SceneData::sceneColor, entt::as_ref_t>("sceneColor"_hs);

    entt::meta_factory<DummyComponent>()
        .type("DummyComponent"_hs)  // Register type name
        .data<&DummyComponent::value, entt::as_ref_t>("value"_hs);
    
    // Reflect NameComponent
    entt::meta_factory<NameComponent>()
        .type("NameComponent"_hs)  // Register type name
        .data<&NameComponent::name, entt::as_ref_t>("name"_hs);
}
