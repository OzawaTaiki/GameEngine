#pragma once

#include <Features/Event/EventTypeRegistry.h>

#include <string>

class GameEvent
{

public:
    GameEvent(const std::string& eventType) : eventType_(eventType),eventTypeID_(0) {}
    ~GameEvent() = default;

    const std::string& GetEventType() const { return eventType_; }

    uint32_t GetEventTypeID() const { return eventTypeID_; }
    uint32_t SetEventType(const std::string& eventType) { eventType_ = eventType; }

private:

    std::string eventType_ = "EventType";
    uint32_t eventTypeID_ = 0;
};