#include "EventManager.h"

EventManager* EventManager::GetInstance()
{
    static EventManager instance;
    return &instance;
}

void EventManager::AddEventListener(const std::string _eventType, iEventListener* _listener)
{
    if (_listener == nullptr)
        return;

    uint32_t eventTypeID = eventTypeRegistry_->GetEventTypeId(_eventType);

    listeners_[eventTypeID].push_back(_listener);

}

void EventManager::RemoveEventListener(const std::string _eventType, iEventListener* _listener)
{
    if (_listener == nullptr)
        return;

    uint32_t eventTypeID = eventTypeRegistry_->GetEventTypeId(_eventType);

    auto& listenerList = listeners_[eventTypeID];

    auto it = std::remove(listenerList.begin(), listenerList.end(), _listener);
    if (it != listenerList.end())
    {
        listenerList.erase(it, listenerList.end());
    }
}

void EventManager::DispatchEvent(const GameEvent& _event)
{
    uint32_t eventTypeID = _event.GetEventTypeID();

    auto it = listeners_.find(eventTypeID);

    if (it != listeners_.end())
    {
        for (auto& listener : it->second)
        {
            listener->OnEvent(_event);
        }
    }
}
