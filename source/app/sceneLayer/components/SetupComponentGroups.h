#pragma once

#include "Components.h"
#include <entt/entt.hpp>

void SetupComponentGroups(entt::registry& registry)
{
	// Transform Hierarchy Group - Most frequently accessed during UpdateMatrices()
	// Groups Transform, LocalToWorld, Parent, and Children for cache-friendly iteration
	// This ensures all transform-related data is packed together in memory
	(void)registry.group<Transform, LocalToWorld, Parent, Children>();

	//// Animated Skeleton Group - Used during animation updates
	//// Groups skeleton data with animation state for efficient animation processing
	//// Optimizes FbxPlayer::Update() and UpdateSkeletonMatrices()
	//(void)registry.group<FBXSkeletonComponent>(entt::get<FBXAnimationComponent, LocalToWorld>);
	//
	//// Skinned Mesh Group - Used for skinned mesh rendering
	//// Groups mesh, skin weights, and transforms for efficient GPU skinning preparation
	//// Future optimization for skinned mesh rendering pipeline
	//(void)registry.group<MeshComponent>(entt::get<FBXSkinComponent, LocalToWorld>);
}