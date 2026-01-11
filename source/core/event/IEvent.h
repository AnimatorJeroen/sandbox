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

        inline void DeleteAllocationIfRequired() const { 
            if (allocationPtr) 
                delete allocationPtr; 
        }

    protected:

        void* allocationPtr = nullptr; // For custom memory allocation trackign if needed

        inline void HeapAllocateData(char*& target, const char* data, size_t size)
        {
            target = (char*)malloc(size);
            std::memcpy(target, data, size);
            allocationPtr = target;
        }
    };
}

