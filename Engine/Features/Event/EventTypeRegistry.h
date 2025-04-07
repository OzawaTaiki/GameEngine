#pragma once

#include <string>
#include <map>
#include <cstdint>

/// <summary>
/// イベントタイプのIDを管理するシングルトンクラス
/// </summary>
class EventTypeRegistry
{
public:
    static EventTypeRegistry* GetInstance();
    EventTypeRegistry() = default;
    ~EventTypeRegistry() = default;

    uint32_t GetEventTypeId(const std::string& _eventType);

    const std::string& GetEventTypeName(uint32_t _eventTypeId) const;

private:
    std::map<std::string, uint32_t> eventTypes_;
};


