#pragma once
#include <core/event/IEvent.h>
#include <string>
#include <entt/entt.hpp>	

class RequestLoadSceneEvent : public Core::IEvent
{
	public:
		char* filepath;
		SET_EVENT_TYPE_FUNCTIONS(RequestLoadSceneEvent)
		RequestLoadSceneEvent(const std::string& _filepath)
		{
			HeapAllocateData(filepath, _filepath.data(), _filepath.size() + 1);
		}
};

class RequestSetActiveSceneEvent : public Core::IEvent
{
	public:
		size_t sceneIndex;
		SET_EVENT_TYPE_FUNCTIONS(RequestSetActiveSceneEvent)
		RequestSetActiveSceneEvent(size_t index) : sceneIndex(index) {}
};

class RequestCloseSceneEvent : public Core::IEvent
{
	public:
		size_t sceneIndex;
		SET_EVENT_TYPE_FUNCTIONS(RequestCloseSceneEvent)
		RequestCloseSceneEvent(size_t index) : sceneIndex(index) {}
};

class OnChangeActiveSceneEvent : public Core::IEvent
{
	public:
		SET_EVENT_TYPE_FUNCTIONS(OnChangeActiveSceneEvent)
		OnChangeActiveSceneEvent() = default;
};

class OnDestroySceneEvent : public Core::IEvent
{
public:
	SET_EVENT_TYPE_FUNCTIONS(OnDestroySceneEvent)
		OnDestroySceneEvent(entt::registry* reg) : registry(reg) {}
	entt::registry* registry;
};
