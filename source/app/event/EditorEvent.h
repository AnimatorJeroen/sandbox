#pragma once
#include <core/event/IEvent.h>
#include <string>

class EditorRequestSaveSceneEvent : public Core::IEvent
{
	public:
		std::string filepath;
		SET_EVENT_TYPE_FUNCTIONS(EditorRequestSaveSceneEvent)
		EditorRequestSaveSceneEvent(const std::string& filepath) : filepath(filepath) {}
};

class EditorRequestLoadSceneEvent : public Core::IEvent
{
	public:
		std::string filepath;
		SET_EVENT_TYPE_FUNCTIONS(EditorRequestLoadSceneEvent)
		EditorRequestLoadSceneEvent(const std::string& filepath) : filepath(filepath) {}
};

class EditorSceneReloadedEvent : public Core::IEvent
{
	public:
		SET_EVENT_TYPE_FUNCTIONS(EditorSceneReloadedEvent)
		EditorSceneReloadedEvent() = default;
};