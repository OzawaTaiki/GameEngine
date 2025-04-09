#pragma once
#include <Features/Event/EventTypeRegistry.h>
#include <Features/Event/EventListener.h>
#include <Features/Event/GameEvent.h>

#include <string>
#include <map>
#include <vector>

class EventManager
{
public:
    static EventManager* GetInstance();

    EventManager() : eventTypeRegistry_(EventTypeRegistry::GetInstance()) {}
    ~EventManager() = default;

    void AddEventListener(const std::string _eventType, iEventListener* _listener);

    void RemoveEventListener(const std::string _eventType, iEventListener* _listener);

    void DispatchEvent(const GameEvent& _event);

private:

    std::map<uint32_t, std::vector<iEventListener*>> listeners_;

    EventTypeRegistry* eventTypeRegistry_;

};