#pragma once
#include "pch.h"
#include "app/sceneLayer/types/Types.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// a DOTS style component system

// Name component for entities
struct NameComponent {
    String64 name;

    NameComponent() = default;
    NameComponent(const std::string& n) : name(n) {}
};

struct Transform {
    vec3 Position{};
	vec3 Rotation{};
    vec3 Scale{ 1.f, 1.f, 1.f };
};

// Parent component - stores the UUID of the parent entity
// This is the SOURCE OF TRUTH for the entity hierarchy
// Uses UUID instead of entt::entity for lifetime safety
struct Parent {
	Core::UUID parentUUID{ 0 };

	Parent() = default;
	Parent(Core::UUID uuid) : parentUUID(uuid) {}
	
	bool HasParent() const { return parentUUID.value != 0; }
};


// Camera component for camera entities
struct CameraComponent {
	vec3 position{0.0f, 5.0f, 10.0f};
	vec3 target{0.0f, 0.0f, 0.0f};
	vec3 up{0.0f, 1.0f, 0.0f};
	float fov = 45.0f;
	float nearPlane = 0.01f;
	float farPlane = 10000.0f;

	CameraComponent() = default;
};

// Mesh component for storing mesh data
struct MeshComponent {
	String64 filepath;
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	std::vector<uint32_t> indices;
	
	MeshComponent() = default;
};

// === FBX-SPECIFIC COMPONENTS ===
// These components store raw FBX data and are kept isolated from general scene logic

// Bone data structure
struct FBXBone {
	mat4 offsetMatrix;     // Transforms from mesh space to bone space (inverse bind pose - constant)
	mat4 localRestTransform;
	
	FBXBone() = default;
};

// Vertex skinning data
struct FBXVertexWeight {
	int boneIndex = -1;    // Index into FBXSkeletonComponent::bones
	float weight = 0.0f;
	
	FBXVertexWeight() = default;
	FBXVertexWeight(int idx, float w) : boneIndex(idx), weight(w) {}
};

// Skeleton structure containing bone hierarchy
struct FBXSkeletonComponent {
	// Runtime data (not serialized, not reflected)
	std::vector<entt::entity> bones;
	FBXSkeletonComponent() = default;
};

// Skin component - stores bone weights for mesh deformation
struct FBXSkinComponent {
	// For each vertex, store up to 4 bone influences (standard for real-time rendering)
	std::vector<std::array<FBXVertexWeight, 4>> vertexWeights;
	int skeletonEntityIndex = -1;  // Reference to entity with FBXSkeletonComponent
	
	FBXSkinComponent() = default;
};

// Animation keyframe for a single bone
struct FBXPositionKey {
	double time;
	vec3 value;
	
	FBXPositionKey() = default;
	FBXPositionKey(double t, const vec3& v) : time(t), value(v) {}
};

struct FBXRotationKey {
	double time;
	glm::quat value;
	
	FBXRotationKey() = default;
	FBXRotationKey(double t, const glm::quat& v) : time(t), value(v) {}
};

struct FBXScaleKey {
	double time;
	vec3 value;
	
	FBXScaleKey() = default;
	FBXScaleKey(double t, const vec3& v) : time(t), value(v) {}
};

// Animation channel for a single bone
struct FBXAnimationChannel {
	int clipIndex = -1;
	std::vector<FBXPositionKey> positionKeys;
	std::vector<FBXRotationKey> rotationKeys;
	std::vector<FBXScaleKey> scaleKeys;
	
	// Runtime data (not serialized, not reflected)
	int boneIndex = -1;

	FBXAnimationChannel() = default;
};

// Container for multiple animation channels (one per clip) on a single bone
struct FBXAnimationChannels {
	std::vector<FBXAnimationChannel> channels;
	
	FBXAnimationChannels() = default;
};

// Complete animation clip
struct FBXAnimationClip {
	String64 name;
	double duration;           // Duration in ticks
	double ticksPerSecond;     // Animation speed

	// Runtime data (not serialized, not reflected)
	std::vector<FBXAnimationChannel*> channels;

	FBXAnimationClip() = default;
};

// Animation component - stores all animation clips for a model
struct FBXAnimationComponent {
	std::vector<FBXAnimationClip> clips;
	int activeClipIndex = 0;
	double currentTime = 0.0;
	bool isPlaying = false;
	bool loop = true;
	
	FBXAnimationComponent() = default;
};

// SceneData component - uniquely identifies the scene root entity
struct SceneData {
	float sceneColor = 0.f;
	String64 _name;
};

// LocalToWorld component - stores the computed world transformation matrix
struct LocalToWorld {
	mat4 Value{ 1.0f };

	LocalToWorld() = default;
	LocalToWorld(const mat4& matrix) : Value(matrix) {}
};


///////////Runtime components (not serialized and not reflected, used for runtime logic and editor functionality)

// Children component - stores entity handles of child entities
// This is COMPUTED/CACHED data that gets recalculated when hierarchy changes
// Uses entt::entity for efficient direct access (will be rebuilt when hierarchy changes)
struct Children {

	// Runtime data (not serialized, not reflected)
	std::vector<entt::entity> children;

	Children() = default;
	Children(const std::vector<entt::entity>& childList) : children(childList) {}

	size_t Count() const { return children.size(); }
	bool HasChildren() const { return !children.empty(); }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Define all component types here as a tuple
// Add ALL your component types to this list, to be registered with the Applicator
using AppComponentTypes = std::tuple<
    Core::UUID,
    Transform,
    NameComponent,
	LocalToWorld,
    CameraComponent,
    MeshComponent,
    FBXSkeletonComponent,
    FBXSkinComponent,
    FBXAnimationComponent,
	FBXAnimationChannels,
	FBXAnimationChannel,
	FBXBone,
	FBXVertexWeight,
	FBXPositionKey,
	FBXRotationKey,
	FBXScaleKey,
    SceneData,  // Scene root entity marker + metadata
    Parent      // Hierarchy: parent reference (source of truth)
>;

