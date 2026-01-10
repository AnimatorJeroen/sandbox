#pragma once
#include "core/event/IEvent.h"

namespace Core
{
	// GLFW modifier bit flags
	constexpr int MOD_SHIFT = 0x0001;
	constexpr int MOD_CONTROL = 0x0002;
	constexpr int MOD_ALT = 0x0004;
	constexpr int MOD_SUPER = 0x0008;
	constexpr int MOD_CAPS_LOCK = 0x0010;
	constexpr int MOD_NUM_LOCK = 0x0020;

	class KeyDownEvent : public Core::IEvent
	{
	public:
		int key;
		bool repeated;
		int mods; // GLFW modifier flags (Shift, Ctrl, Alt, Super)
		KeyDownEvent(int _key, bool _repeated, int _mods = 0) : key(_key), repeated(_repeated), mods(_mods) {}
		SET_EVENT_TYPE_FUNCTIONS(KeyDownEvent)
	};

	class KeyUpEvent : public Core::IEvent
	{
	public:
		int key;
		int mods; // GLFW modifier flags (Shift, Ctrl, Alt, Super)
		KeyUpEvent(int _key, int _mods = 0) : key(_key), mods(_mods) {}
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