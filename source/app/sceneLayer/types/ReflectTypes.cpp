#include "ReflectTypes.hpp"

#include "../vendor/include/entt/entt.hpp"
#include "../vendor/include/entt/meta/meta.hpp"
#include "Types.hpp"
#include <app/sceneLayer/Scene.h>

// ----- Meta registration (call once before usage)
void ReflectTypes() {
    using namespace entt::literals;

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

    entt::meta_factory<Scene>()
        .data<&Scene::sceneColor>("sceneColor"_hs);
}
