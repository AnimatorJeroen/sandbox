#pragma once
#include <core/event/IEvent.h>
#include <string>

class RequestSaveSceneEvent : public Core::IEvent
{
	public:
		std::string filepath;
		SET_EVENT_TYPE_FUNCTIONS(RequestSaveSceneEvent)
		RequestSaveSceneEvent(const std::string& filepath) : filepath(filepath) {}
};

class RequestLoadSceneEvent : public Core::IEvent
{
	public:
		std::string filepath;
		SET_EVENT_TYPE_FUNCTIONS(RequestLoadSceneEvent)
		RequestLoadSceneEvent(const std::string& filepath) : filepath(filepath) {}
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