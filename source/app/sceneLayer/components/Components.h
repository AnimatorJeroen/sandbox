#pragma once
#include "pch.h"
#include "app/sceneLayer/types/Types.h"

// Name component for entities
struct NameComponent {
    String64 name;

    NameComponent() = default;
    NameComponent(const std::string& n) : name(n) {}
};

struct Transform {
    vec4 Position{};
    vec4 Scale{ 1.f, 1.f, 1.f, 1.f };
	mat4 Matrix{ 1.0f };
};

// SceneData component - uniquely identifies the scene root entity
struct SceneData {
	float sceneColor = 0.f;
	String64 _name;
	
	// Camera settings are part of scene data
	glm::vec3 cameraPosition{0.0f, 5.0f, 10.0f};
	glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};
	float cameraFOV = 45.0f;
};

// Define all component types here as a tuple
// Add ALL your component types to this list, to be registered with the Applicator
using AppComponentTypes = std::tuple<
    Core::UUID,
    Transform,
    NameComponent,
    SceneData  // Scene root entity marker + metadata
>;