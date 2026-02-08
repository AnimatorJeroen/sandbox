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

    // Reflect vec2 fields with as_ref policy
    entt::meta_factory<vec2>()
        .type("vec2"_hs)  // Register type name
        .data<&vec2::x, entt::as_ref_t>("x"_hs)
        .data<&vec2::y, entt::as_ref_t>("y"_hs);

    // Reflect vec3 fields with as_ref policy
    entt::meta_factory<vec3>()
        .type("vec3"_hs)  // Register type name
        .data<&vec3::x, entt::as_ref_t>("x"_hs)
        .data<&vec3::y, entt::as_ref_t>("y"_hs)
        .data<&vec3::z, entt::as_ref_t>("z"_hs);

    // Reflect mat4
    entt::meta_factory<mat4>()
        .type("mat4"_hs);  // Register type name

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

    // Reflect CameraComponent
    entt::meta_factory<CameraComponent>()
        .type("CameraComponent"_hs)
        .data<&CameraComponent::position, entt::as_ref_t>("position"_hs)
        .data<&CameraComponent::target, entt::as_ref_t>("target"_hs)
        .data<&CameraComponent::up, entt::as_ref_t>("up"_hs)
        .data<&CameraComponent::fov, entt::as_ref_t>("fov"_hs)
        .data<&CameraComponent::nearPlane, entt::as_ref_t>("nearPlane"_hs)
        .data<&CameraComponent::farPlane, entt::as_ref_t>("farPlane"_hs);

    // Reflect MeshComponent
    entt::meta_factory<MeshComponent>()
        .type("MeshComponent"_hs)
        .data<&MeshComponent::filepath, entt::as_ref_t>("filepath"_hs);
        // Note: vector members (vertices, normals, texCoords, indices) not individually reflected

    // ===== FBX-SPECIFIC COMPONENT REFLECTION =====

    // Reflect FBXBone
    entt::meta_factory<FBXBone>()
        .type("FBXBone"_hs)
        .data<&FBXBone::name, entt::as_ref_t>("name"_hs)
        .data<&FBXBone::offsetMatrix, entt::as_ref_t>("offsetMatrix"_hs);

    // Reflect FBXVertexWeight
    entt::meta_factory<FBXVertexWeight>()
        .type("FBXVertexWeight"_hs)
        .data<&FBXVertexWeight::boneIndex, entt::as_ref_t>("boneIndex"_hs)
        .data<&FBXVertexWeight::weight, entt::as_ref_t>("weight"_hs);

    // Reflect FBXSkeletonComponent
    entt::meta_factory<FBXSkeletonComponent>()
        .type("FBXSkeletonComponent"_hs)
        .data<&FBXSkeletonComponent::skeletonName, entt::as_ref_t>("skeletonName"_hs);
        // Note: bones vector not individually reflected

    // Reflect FBXSkinComponent
    entt::meta_factory<FBXSkinComponent>()
        .type("FBXSkinComponent"_hs)
        .data<&FBXSkinComponent::skeletonEntityIndex, entt::as_ref_t>("skeletonEntityIndex"_hs);
        // Note: vertexWeights vector not individually reflected

    // Reflect FBXPositionKey
    entt::meta_factory<FBXPositionKey>()
        .type("FBXPositionKey"_hs)
        .data<&FBXPositionKey::time, entt::as_ref_t>("time"_hs)
        .data<&FBXPositionKey::value, entt::as_ref_t>("value"_hs);

    // Reflect FBXRotationKey
    entt::meta_factory<FBXRotationKey>()
        .type("FBXRotationKey"_hs)
        .data<&FBXRotationKey::time, entt::as_ref_t>("time"_hs);
        // Note: glm::quat value not individually reflected

    // Reflect FBXScaleKey
    entt::meta_factory<FBXScaleKey>()
        .type("FBXScaleKey"_hs)
        .data<&FBXScaleKey::time, entt::as_ref_t>("time"_hs)
        .data<&FBXScaleKey::value, entt::as_ref_t>("value"_hs);

    // Reflect FBXAnimationChannel
    entt::meta_factory<FBXAnimationChannel>()
        .type("FBXAnimationChannel"_hs)
        .data<&FBXAnimationChannel::clipIndex, entt::as_ref_t>("clipIndex"_hs)
        .data<&FBXAnimationChannel::boneIndex, entt::as_ref_t>("boneIndex"_hs);
        // Note: keyframe vectors not individually reflected

    // Reflect FBXAnimationChannels
    entt::meta_factory<FBXAnimationChannels>()
        .type("FBXAnimationChannels"_hs);
        // Note: channels vector not individually reflected

    // Reflect FBXAnimationClip
    entt::meta_factory<FBXAnimationClip>()
        .type("FBXAnimationClip"_hs)
        .data<&FBXAnimationClip::name, entt::as_ref_t>("name"_hs)
        .data<&FBXAnimationClip::duration, entt::as_ref_t>("duration"_hs)
        .data<&FBXAnimationClip::ticksPerSecond, entt::as_ref_t>("ticksPerSecond"_hs);
        // Note: channels vector not individually reflected

    // Reflect FBXAnimationComponent
    entt::meta_factory<FBXAnimationComponent>()
        .type("FBXAnimationComponent"_hs)
        .data<&FBXAnimationComponent::activeClipIndex, entt::as_ref_t>("activeClipIndex"_hs)
        .data<&FBXAnimationComponent::currentTime, entt::as_ref_t>("currentTime"_hs)
        .data<&FBXAnimationComponent::isPlaying, entt::as_ref_t>("isPlaying"_hs)
        .data<&FBXAnimationComponent::loop, entt::as_ref_t>("loop"_hs);
        // Note: clips vector not individually reflected
}
