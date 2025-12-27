#pragma once
#include "core/event/IEvent.h"

namespace Core
{
	class KeyDownEvent : public Core::IEvent
	{
	public:
		int key;
		bool repeated;
		KeyDownEvent(int _key, bool _repeated) : key(_key), repeated(_repeated) {}
		SET_EVENT_TYPE_FUNCTIONS(KeyDownEvent)
	};

	class KeyUpEvent : public Core::IEvent
	{
		public:
		int key;
		KeyUpEvent(int _key) : key(_key) {}
		SET_EVENT_TYPE_FUNCTIONS(KeyUpEvent)
	};
	
	class KeyCharacterEvent : public Core::IEvent
	{
	public:
		unsigned int character;
		KeyCharacterEvent(unsigned int _character) : character(_character) {}
		SET_EVENT_TYPE_FUNCTIONS(KeyCharacterEvent)
	};
}