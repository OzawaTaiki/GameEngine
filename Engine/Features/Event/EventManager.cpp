#include "EventManager.h"


namespace Engine {

EventManager* EventManager::GetInstance()
{
    static EventManager instance;
    return &instance;
}

void EventManager::AddEventListener(const std::string& _eventType, iEventListener* _listener)
{
    if (_listener == nullptr)
        return;


    listeners_[_eventType].push_back(_listener);

}

void EventManager::RemoveEventListener(const std::string& _eventType, iEventListener* _listener)
{
    if (_listener == nullptr)
        return;


    auto& listenerList = listeners_[_eventType];

    auto it = std::remove(listenerList.begin(), listenerList.end(), _listener);
    if (it != listenerList.end())
    {
        listenerList.erase(it, listenerList.end());
    }
}

void EventManager::DispatchEvent(const GameEvent& _event)
{
    std::string eventType = _event.GetEventType();

    auto it = listeners_.find(eventType);

    if (it != listeners_.end())
    {
        for (auto& listener : it->second)
        {
            listener->OnEvent(_event);
        }
    }
}

} // namespace Engine
