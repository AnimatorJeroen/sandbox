#pragma once

#include <map>
#include <queue>
#include "IEvent.h"
#include <functional>
#include <typeindex>
#include <memory>
#include <cstring>
#include <stdexcept>


#define REGISTER_CALLBACK(eventBus, T, func) eventBus.RegisterEventCallback<T>([this](const auto& e) { this->func((const T&)e); })

namespace Core
{
    class EventBus
    {
    private:

        class QueueEvent
        {
        public:
            static constexpr size_t MAX_EVENT_SIZE = 128;

            std::type_index typeIndex;
            size_t size;
            char data[MAX_EVENT_SIZE];

            QueueEvent(std::type_index _typeIndex, size_t _size, const char* _data)
                : typeIndex(_typeIndex), size(_size)
            {
                if (_size > MAX_EVENT_SIZE)
                    throw std::runtime_error("Event size exceeds maximum allowed size");
                
                std::memcpy(data, _data, _size);
            }
        };

        std::queue<QueueEvent> m_eventQueue;
        std::map<std::type_index, std::vector<std::function<void(const IEvent&)>>> m_eventCallbacks;

    public:

        template<typename TDerivedEvent>
        inline void RegisterEventCallback(const std::function<void(const IEvent&)>& callback)
        {
            std::type_index type = typeid(TDerivedEvent);
            auto elem = m_eventCallbacks.find(type);
            if (elem != m_eventCallbacks.end()) //add to existing
                elem->second.emplace_back(callback);
            else //or create a new entry
                m_eventCallbacks.insert({ type, {callback} });
        }

        template<typename TDerivedEvent>
        inline void PushEvent(const TDerivedEvent& e) 
        { 
            m_eventQueue.emplace(QueueEvent(typeid(TDerivedEvent), sizeof(TDerivedEvent), (const char*)&e)); 
        }

        inline void HandleEvents()
        {
            while (!m_eventQueue.empty())
            {
                const auto& e = m_eventQueue.front();

                if (auto elem = m_eventCallbacks.find(e.typeIndex); elem != m_eventCallbacks.end())
                    for (const auto& callback : elem->second)
                        callback(*(const IEvent*)e.data);

                m_eventQueue.pop();
            }
        }
    };
}