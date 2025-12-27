#pragma once
#include <typeindex>

#define SET_EVENT_TYPE_FUNCTIONS(typeName) const char* GetName() const override {return #typeName; }

namespace Core
{
    class IEvent
    {
    public:

        ~IEvent() = default;

        virtual const char* GetName() const = 0;

        template<typename TFunc>
        inline void Dispatch(const TFunc& func) const
        {
            func(*this);
        }
    };
}

