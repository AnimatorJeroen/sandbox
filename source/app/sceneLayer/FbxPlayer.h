#pragma once
#include "pch.h"
#include <entt/entt.hpp>
#include "components/Components.h"
#include "core/Logger.h"

class FbxPlayer
{
public:
	FbxPlayer() = default;
	~FbxPlayer() = default;

	// Update all FBX animations in the scene
	void Update(entt::registry& registry, float deltaTime)
	{
		// Find all entities with FBXAnimationComponent
		auto animView = registry.view<FBXAnimationComponent>();
		
		for (auto entity : animView)
		{
			auto& animComp = registry.get<FBXAnimationComponent>(entity);
			
			animComp.isPlaying = true;
			animComp.activeClipIndex = 1;
			// Only update if animation is playing and has clips
			if (!animComp.isPlaying || animComp.clips.empty())
				continue;
			
			// Make sure active clip index is valid
			if (animComp.activeClipIndex < 0 || 
			    animComp.activeClipIndex >= static_cast<int>(animComp.clips.size()))
			{
				animComp.activeClipIndex = 0;
			}
			
			// Get the active animation clip
			FBXAnimationClip& clip = animComp.clips[animComp.activeClipIndex];
			
			// Update animation time
			double ticksPerSecond = clip.ticksPerSecond != 0.0 ? clip.ticksPerSecond : 25.0;
			animComp.currentTime += deltaTime * ticksPerSecond;
			
			// Handle looping
			if (animComp.currentTime > clip.duration)
			{
				if (animComp.loop)
				{
					// Loop the animation
					animComp.currentTime = std::fmod(animComp.currentTime, clip.duration);
				}
				else
				{
					// Stop at the end
					animComp.currentTime = clip.duration;
					animComp.isPlaying = false;
				}
			}
			
			// Log animation progress (optional - can be removed for performance)
			if (static_cast<int>(animComp.currentTime * 10) % 10 == 0)
			{
				LOG_TRACE() << "FBX Animation '" << clip.name 
				           << "' Time: " << animComp.currentTime 
				           << "/" << clip.duration;
			}
		}
	}
	
	// Play a specific animation clip on an entity
	void Play(entt::registry& registry, entt::entity entity, int clipIndex = 0, bool loop = true)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		
		if (clipIndex >= 0 && clipIndex < static_cast<int>(animComp.clips.size()))
		{
			animComp.activeClipIndex = clipIndex;
			animComp.currentTime = 0.0;
			animComp.isPlaying = true;
			animComp.loop = loop;
			
			LOG_INFO() << "Playing FBX animation clip " << clipIndex 
			          << ": " << animComp.clips[clipIndex].name;
		}
	}
	
	// Stop animation on an entity
	void Stop(entt::registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		animComp.isPlaying = false;
	}
	
	// Pause animation on an entity
	void Pause(entt::registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		animComp.isPlaying = false;
	}
	
	// Resume animation on an entity
	void Resume(entt::registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		
		if (!animComp.clips.empty())
		{
			animComp.isPlaying = true;
		}
	}
};
