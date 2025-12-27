#pragma once
#include "core/event/IEvent.h"

namespace Core
{
	class MouseDownEvent : public Core::IEvent
	{
	public:
		unsigned int identifier;
		MouseDownEvent(unsigned int _identifier = 0) : identifier(_identifier) {}

		SET_EVENT_TYPE_FUNCTIONS(MouseDownEvent)
	};

	class MouseUpEvent : public Core::IEvent
	{
	public:
		unsigned int identifier;
		MouseUpEvent(unsigned int _identifier = 0) : identifier(_identifier) {}

		SET_EVENT_TYPE_FUNCTIONS(MouseUpEvent)
	};

	class MouseMoveEvent : public Core::IEvent
	{
	public:
		double posX, posY;
		MouseMoveEvent(double x, double y) : posX(x), posY(y) {}

		SET_EVENT_TYPE_FUNCTIONS(MouseMoveEvent)
	};
	
	class MouseScrollEvent : public Core::IEvent
	{
	public:
		double scrollX, scrollY;
		MouseScrollEvent(double x, double y) : scrollX(x), scrollY(y) {}

		SET_EVENT_TYPE_FUNCTIONS(MouseScrollEvent)
	};

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
}