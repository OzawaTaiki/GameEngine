#include "EventTypeRegistry.h"

#include <stdexcept>

EventTypeRegistry* EventTypeRegistry::GetInstance()
{
    static EventTypeRegistry instance;
    return &instance;
}

uint32_t EventTypeRegistry::GetEventTypeId(const std::string& _eventType)
{
    if (eventTypes_.contains(_eventType))
    {
        return eventTypes_[_eventType];
    }
    else
    {
        uint32_t newId = static_cast<uint32_t>(eventTypes_.size());
        eventTypes_[_eventType] = newId;
        return newId;
    }
}

const std::string& EventTypeRegistry::GetEventTypeName(uint32_t _eventTypeId) const
{
    for (const auto& pair : eventTypes_)
    {
        if (pair.second == _eventTypeId)
        {
            return pair.first;
        }
    }

    throw std::runtime_error("Event type ID not found");
    return std::string();
}
