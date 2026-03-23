#pragma once

#include <string>
#include <vector>

namespace Engine
{

enum class SoundEventType
{
    Play,
    Stop,
    Pause,
    Resume,
    SetVolume
};

struct SoundEventAction
{
    SoundEventType type;
    std::string soundId; // Play の場合に使用
    float volume;       // SetVolume の場合に使用
    bool loop;          // Play の場合に使用
};


struct SoundEventDef
{
    std::string name;   // イベント名
    std::vector<SoundEventAction> actions; // イベントに紐づくアクションのリスト
};


} // namespace Engine
