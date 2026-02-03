#pragma once
#include <entt/entt.hpp>
#include <core/UUID.h>
#include <glm/gtc/type_ptr.hpp>

// TransformBundle - Helper struct to access transform-related components together
struct TransformBundle {
    vec3& Position;
    vec3& Rotation;
    vec3& Scale;
    mat4& LocalToWorld;
    entt::entity parentEntity = entt::null;
    std::vector<entt::entity> ChildrenEntities;

    //computed properties
    vec3 worldPosition;
    vec3 worldRotation;
    vec3 worldScale;


    TransformBundle() : Position(vec3()),
        Rotation(vec3()),
        Scale(vec3()),
        LocalToWorld(mat4()) {
    }
    TransformBundle(const TransformBundle& other) = default;

    TransformBundle(vec3& position, vec3& rotation, vec3& scale,
        mat4& localToWorld, entt::entity parent = entt::null, const std::vector<entt::entity>& childrenEntities = {}) :
        Position(position),
        Rotation(rotation),
        Scale(scale),
        LocalToWorld(localToWorld),
        parentEntity(parent),
        ChildrenEntities(childrenEntities)
    {
        // Decompose the LocalToWorld matrix to get world space transform
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localToWorld),
            glm::value_ptr(worldPosition), glm::value_ptr(worldRotation), glm::value_ptr(worldScale));
    }

    static TransformBundle Null() { return TransformBundle(); }
};
