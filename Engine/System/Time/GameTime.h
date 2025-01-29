#pragma once

#include <System/Time/GameTimeChannel.h>

#include <map>
#include <string>

class Time;
class GameTime
{
public:

    static GameTime* GetInstance();
    static GameTimeChannel& GetChannel(const std::string& _name) { return GetInstance()->channels_[_name]; }

    void Initialize();
    void Update();


    void CreateChannel(const std::string& _name);
    void EraseChannel(const std::string& _name);

    static void SetDefaultFramerate(float _framerate);
    static void SetDeltaTimeFixed(bool _isFixed);

    static float GetUnScaleDeltaTime_float();
    static double GetUnScaleDeltaTime_double();

    static bool IsFixedDeltaTime();

private:

    std::map<std::string, GameTimeChannel> channels_;


private:
    GameTime();
    ~GameTime() = default;
public:
    GameTime(const GameTime&) = delete;
    GameTime& operator=(const GameTime&) = delete;


};
