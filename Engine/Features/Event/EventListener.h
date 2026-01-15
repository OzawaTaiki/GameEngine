#pragma once

#include <Features/Event/GameEvent.h>

// interface


namespace Engine {

class iEventListener
{
public:
    virtual ~iEventListener() = default;

    /// <summary>
    /// イベントを受け取る
    /// </summary>
    /// <param name="event">イベント</param>
    virtual void OnEvent(const GameEvent& _event) = 0;
};

} // namespace Engine
