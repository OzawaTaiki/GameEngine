#pragma once

#include <cstdint>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

class Time
{
public:

    static void Initialize();
    static void Update();

    static double GetDeltaTime() { return deltaTime_; }
    static double GetTotalTime() { return totalTime_; }
    static double GetFramerate() { return framerate_; }
    static double GetCurrentTime();

private:

    // deltaTimeの計算間隔
    static double deltaTimeUpdateInterval_;
    static int32_t frameCount_;

    static double deltaTime_;
    static double totalTime_;

    static double prevTime_;
    static double currentTime_;
    static double framerate_;


};
