#pragma once
#include <Features/Event/EventListener.h>
#include <Features/Event/GameEvent.h>

#include <string>
#include <map>
#include <vector>


namespace Engine {

class EventManager
{
public:
    static EventManager* GetInstance();

    EventManager() = default;
    ~EventManager() = default;

    void AddEventListener(const std::string& _eventType, iEventListener* _listener);

    void RemoveEventListener(const std::string& _eventType, iEventListener* _listener);

    void DispatchEvent(const GameEvent& _event);

private:

    std::map<std::string, std::vector<iEventListener*>> listeners_;


};

} // namespace Engine
