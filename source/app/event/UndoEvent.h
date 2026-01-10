#pragma once
#include <core/event/IEvent.h>

class RequestUndoEvent : public Core::IEvent {
public:
    SET_EVENT_TYPE_FUNCTIONS(RequestUndoEvent)
    RequestUndoEvent() = default;
};

class RequestRedoEvent : public Core::IEvent {
public:
    SET_EVENT_TYPE_FUNCTIONS(RequestRedoEvent)
    RequestRedoEvent() = default;
};
