#pragma once

#include <Systems/Time/Time.h>

class GameTimeChannel
{
public:
    GameTimeChannel();

    void Initialize();
    void Update();

    void ApplyHitStop(float _time) { hitStopTime_ = _time; }


    void SetGameSpeed(float _speed) { gameSpeed_ = _speed; }
    void SetPaused(bool _isPaused) { isPaused_ = _isPaused; }

    template<typename T>
    T GetDeltaTime() { return static_cast<T>(deltaTime_); }



private:

    double deltaTime_ = 1.0f / 60.0f;

    float gameSpeed_ = 1.0f;
    bool isPaused_ = false;
    float hitStopTime_ = 0.0f;
};