#pragma once
#include <entt/entt.hpp>
#include <core/UUID.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

// TransformBundle - Helper struct to access transform-related components together
class TransformBundle {

public:
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

private:
	entt::registry* _registry;

public:
    TransformBundle() : Position(vec3()),
        Rotation(vec3()),
        Scale(vec3()),
        LocalToWorld(mat4()) {
    }
    TransformBundle(const TransformBundle& other) = default;

    TransformBundle(entt::registry* registry, vec3& position, vec3& rotation, vec3& scale,
        mat4& localToWorld, entt::entity parent = entt::null, const std::vector<entt::entity>& childrenEntities = {}) :
		_registry(registry),
        Position(position),
        Rotation(rotation),
        Scale(scale),
        LocalToWorld(localToWorld),
        parentEntity(parent),
        ChildrenEntities(childrenEntities)
    {
        // Decompose the LocalToWorld matrix to get world space transform
        glm::quat orientation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(localToWorld, worldScale, orientation, worldPosition, skew, perspective);
        
        // Convert quaternion to Euler angles in degrees
        worldRotation = glm::degrees(glm::eulerAngles(orientation));
    }

    mat4 GetParentMatrix() const {
        if (parentEntity != entt::null) {
			return _registry->get<::LocalToWorld>(parentEntity).Value;
        }
        return mat4(1.0f);
    }

    static TransformBundle Null() { return TransformBundle(); }
};
