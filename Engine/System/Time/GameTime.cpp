#include <System/Time/GameTime.h>
#include <System/Time/Time.h>

GameTime* GameTime::GetInstance()
{
    static GameTime instance;
    return &instance;
}

GameTime::GameTime()
{
    channels_["default"] = GameTimeChannel();
}

void GameTime::Initialize()
{
    for (auto& channel : channels_)
    {
        channel.second.Initialize();
    }
}

void GameTime::Update()
{
    for (auto& channel : channels_)
    {
        channel.second.Update();
    }

}

void GameTime::CreateChannel(const std::string& _name)
{
    if (channels_.find(_name) == channels_.end())
    {
        channels_[_name] = GameTimeChannel();
        channels_[_name].Initialize();
    }
}

void GameTime::EraseChannel(const std::string& _name)
{
    if (channels_.find(_name) != channels_.end())
        channels_.erase(_name);
}

void GameTime::SetDefaultFramerate(float _framerate)
{
    Time::SetDefaultFramerate(_framerate);
}

void GameTime::SetDeltaTimeFixed(bool _isFixed)
{
    Time::SetDeltaTimeFixed(_isFixed);
}

float GameTime::GetUnScaleDeltaTime_float()
{
    return Time::GetDeltaTime<float>();
}

double GameTime::GetUnScaleDeltaTime_double()
{
    return Time::GetDeltaTime<double>();
}

bool GameTime::IsFixedDeltaTime()
{
    return Time::IsDeltaTimeFixed();
}
