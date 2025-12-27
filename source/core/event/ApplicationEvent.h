#pragma once
#include "core/event/IEvent.h"

namespace Core
{
	class ApplicationCloseEvent : public Core::IEvent
	{
	public:
		SET_EVENT_TYPE_FUNCTIONS(ApplicationCloseEvent)
	};

	class ApplicationPauseEvent : public Core::IEvent
	{
	public:
		SET_EVENT_TYPE_FUNCTIONS(ApplicationPauseEvent)
	};

	class WindowResizeEvent : public Core::IEvent
	{
	public:
		int Width, Height;
		WindowResizeEvent(int width, int height) 
			: Width(width), Height(height) {}
		SET_EVENT_TYPE_FUNCTIONS(WindowResizeEvent)
	};

	class WindowCloseEvent : public Core::IEvent
	{
		public:
			SET_EVENT_TYPE_FUNCTIONS(WindowCloseEvent)
	};
}