#pragma once

#include <Features/Event/EventTypeRegistry.h>

#include <string>

class GameEvent
{

public:
    GameEvent(const std::string& eventType) : eventType_(eventType) , eventTypeID_(EventTypeRegistry::GetInstance()->GetEventTypeId(eventType)) {}
    ~GameEvent() = default;

    const std::string& GetEventType() const { return eventType_; }

    uint32_t GetEventTypeID() const { return eventTypeID_; }
    void SetEventType(const std::string& eventType) {
        eventType_ = eventType;
        eventTypeID_ = EventTypeRegistry::GetInstance()->GetEventTypeId(eventType);
    }

private:

    std::string eventType_ = "EventType";
    uint32_t eventTypeID_ = 0;
};