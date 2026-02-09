#pragma once
#include <entt/entt.hpp>
#include "components/Components.h"

class Scene;

class FbxPlayer
{
public:
	FbxPlayer() = default;
	~FbxPlayer() = default;

	// Update all FBX animations in the scene
	void Update(Scene& scene, float deltaTime);
private:
	// Apply animation to skeleton bones
	void ApplyAnimationToSkeleton(Core::Registry& registry, Entity animEntity,
		const FBXAnimationClip& clip, double currentTime);

	// Interpolate position between keyframes
	vec3 InterpolatePosition(const FBXAnimationChannel& channel, double time) const;
	
	// Interpolate rotation between keyframes
	glm::quat InterpolateRotation(const FBXAnimationChannel& channel, double time) const;
	
	// Interpolate scale between keyframes
	vec3 InterpolateScale(const FBXAnimationChannel& channel, double time) const;

public:
	// Play a specific animation clip on an entity
	void Play(Core::Registry& registry, entt::entity entity, int clipIndex = 0, bool loop = true);
	
	// Stop animation on an entity
	void Stop(Core::Registry& registry, entt::entity entity);
	
	// Pause animation on an entity
	void Pause(Core::Registry& registry, entt::entity entity);
	
	// Resume animation on an entity
	void Resume(Core::Registry& registry, entt::entity entity);
};
