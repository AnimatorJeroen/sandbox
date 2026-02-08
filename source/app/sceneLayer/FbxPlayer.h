#pragma once
#include "pch.h"
#include <entt/entt.hpp>
#include "components/Components.h"
#include "core/Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class FbxPlayer
{
public:
	FbxPlayer() = default;
	~FbxPlayer() = default;

	// Update all FBX animations in the scene
	void Update(Core::Registry& registry, float deltaTime)
	{
		// Find all entities with FBXAnimationComponent
		auto animView = registry.view<FBXAnimationComponent>();
		
		for (auto animEntity : animView)
		{
			auto& animComp = registry.get<FBXAnimationComponent>(animEntity);
			
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
			
			 // Find the skeleton entity and apply animation
			ApplyAnimationToSkeleton(registry, animEntity, clip, animComp.currentTime);
		}
	}
	
private:
	// Apply animation to skeleton bones
	void ApplyAnimationToSkeleton(Core::Registry& registry, entt::entity animEntity, 
	                              const FBXAnimationClip& clip, double currentTime)
	{
		if(!registry.all_of<FBXSkeletonComponent>(animEntity))
			return;

		auto& skeleton = registry.get<FBXSkeletonComponent>(animEntity);

		for (auto& bone : skeleton.bones)
			bone.localTransform = bone.localRestTransform;

		// Apply animation channels to bones
		for (const auto& channel : clip.channels)
		{
			// Find the bone index by name
			int boneIndex = channel.boneIndex;//FindBoneIndex(skeleton, channel.boneName);
			if (boneIndex < 0)
				continue;
				
			FBXBone& bone = skeleton.bones[boneIndex];
				

			//vec3 position;
			//glm::quat rotation;
			//vec3 scale;
			//glm::vec3 skew;
			//glm::vec4 perspective;
			//glm::decompose(bone.localRestTransform, scale, rotation, position, skew, perspective);


			vec3 positionAdditive = InterpolatePosition(channel, currentTime);
			glm::quat rotationAdditive = InterpolateRotation(channel, currentTime);
			vec3 scaleAdditive = InterpolateScale(channel, currentTime);

			mat4 translationMat = glm::translate(mat4(1.0f), positionAdditive);
			mat4 rotationMat = glm::toMat4(rotationAdditive);
			mat4 scaleMat = glm::scale(mat4(1.0f), scaleAdditive);

			bone.localTransform = translationMat * rotationMat * scaleMat;
		}

	}
	
	// Find bone index by name
	int FindBoneIndex(const FBXSkeletonComponent& skeleton, const String64& boneName) const
	{
		for (size_t i = 0; i < skeleton.bones.size(); i++)
		{
			// First try exact match
			if (std::strcmp(skeleton.bones[i].name.data, boneName.data) == 0)
				return static_cast<int>(i);
		}
		for (size_t i = 0; i < skeleton.bones.size(); i++)
		{
			// Second try: strip last character from skeleton bone name

			if (skeleton.bones[i].name.length() > boneName.length())
			{
				std::string skeletonBoneName = skeleton.bones[i].name.data;
				std::string strippedName = skeletonBoneName.substr(0, boneName.length());
				if (std::strcmp(strippedName.data(), boneName.data) == 0)
					return static_cast<int>(i);
			}
			else
			{
				std::string skeletonBoneName = boneName.data;
				std::string strippedName = skeletonBoneName.substr(0, skeleton.bones[i].name.length());
				if (std::strcmp(strippedName.data(), skeleton.bones[i].name.data) == 0)
					return static_cast<int>(i);
			}
		}

		// Log when a bone isn't found to help debug name mismatches
		LOG_WARN() << "Animation channel bone '" << boneName.data << "' not found in skeleton";
		return -1;
	}
	
	// Interpolate position between keyframes
	vec3 InterpolatePosition(const FBXAnimationChannel& channel, double time) const
	{
		if (channel.positionKeys.empty())
			return vec3(0.0f);
		
		if (channel.positionKeys.size() == 1 || time <= channel.positionKeys[0].time)
			return channel.positionKeys[0].value;
		
		// Find the two keyframes to interpolate between
		for (size_t i = 0; i < channel.positionKeys.size() - 1; i++)
		{
			if (time >= channel.positionKeys[i].time && time < channel.positionKeys[i + 1].time)
			{
				double t0 = channel.positionKeys[i].time;
				double t1 = channel.positionKeys[i + 1].time;
				float factor = static_cast<float>((time - t0) / (t1 - t0));
				
				return glm::mix(channel.positionKeys[i].value, channel.positionKeys[i + 1].value, factor);
			}
		}
		
		// Return last keyframe if time is past all keyframes
		return channel.positionKeys.back().value;
	}
	
	// Interpolate rotation between keyframes
	glm::quat InterpolateRotation(const FBXAnimationChannel& channel, double time) const
	{

		if (channel.rotationKeys.empty())
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			
		
		if (channel.rotationKeys.size() == 1 || time <= channel.rotationKeys[0].time)
			return channel.rotationKeys[0].value;
		
		// Find the two keyframes to interpolate between
		for (size_t i = 0; i < channel.rotationKeys.size() - 1; i++)
		{
			if (time >= channel.rotationKeys[i].time && time < channel.rotationKeys[i + 1].time)
			{
				double t0 = channel.rotationKeys[i].time;
				double t1 = channel.rotationKeys[i + 1].time;
				float factor = static_cast<float>((time - t0) / (t1 - t0));
				
				return glm::slerp(channel.rotationKeys[i].value, channel.rotationKeys[i + 1].value, factor);
			}
		}
		
		// Return last keyframe if time is past all keyframes
		return channel.rotationKeys.back().value;
	}
	
	// Interpolate scale between keyframes
	vec3 InterpolateScale(const FBXAnimationChannel& channel, double time) const
	{
		if (channel.scaleKeys.empty())
			return vec3(1.0f);
		
		if (channel.scaleKeys.size() == 1 || time <= channel.scaleKeys[0].time)
			return channel.scaleKeys[0].value;
		
		// Find the two keyframes to interpolate between
		for (size_t i = 0; i < channel.scaleKeys.size() - 1; i++)
		{
			if (time >= channel.scaleKeys[i].time && time < channel.scaleKeys[i + 1].time)
			{
				double t0 = channel.scaleKeys[i].time;
				double t1 = channel.scaleKeys[i + 1].time;
				float factor = static_cast<float>((time - t0) / (t1 - t0));
				
				return glm::mix(channel.scaleKeys[i].value, channel.scaleKeys[i + 1].value, factor);
			}
		}
		
		// Return last keyframe if time is past all keyframes
		return channel.scaleKeys.back().value;
	}

public:
	// Play a specific animation clip on an entity
	void Play(Core::Registry& registry, entt::entity entity, int clipIndex = 0, bool loop = true)
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
	void Stop(Core::Registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		animComp.isPlaying = false;
	}
	
	// Pause animation on an entity
	void Pause(Core::Registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FBXAnimationComponent>(entity))
			return;
		
		auto& animComp = registry.get<FBXAnimationComponent>(entity);
		animComp.isPlaying = false;
	}
	
	// Resume animation on an entity
	void Resume(Core::Registry& registry, entt::entity entity)
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
