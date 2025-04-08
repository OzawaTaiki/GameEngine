#pragma once

#include <Features/Event/EventTypeRegistry.h>
#include <Features/Event/EventData.h>


#include <string>

class GameEvent
{

public:
    GameEvent(const std::string& eventType,EventData* _data) : eventType_(eventType) ,data_(_data), eventTypeID_(EventTypeRegistry::GetInstance()->GetEventTypeId(eventType)) {}
    ~GameEvent() = default;

    const std::string& GetEventType() const { return eventType_; }

    EventData* GetData() const { return data_; }

    uint32_t GetEventTypeID() const { return eventTypeID_; }
    void SetEventType(const std::string& eventType) {
        eventType_ = eventType;
        eventTypeID_ = EventTypeRegistry::GetInstance()->GetEventTypeId(eventType);
    }

private:

    std::string eventType_ = "EventType";
    uint32_t eventTypeID_ = 0;
    EventData* data_ = nullptr;
};