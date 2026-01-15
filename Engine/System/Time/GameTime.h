#pragma once

#include <System/Time/GameTimeChannel.h>

#include <map>
#include <string>


namespace Engine {

class Time;
class GameTime
{
public:

    static GameTime* GetInstance();
    static GameTimeChannel& GetChannel(const std::string& _name) { return GetInstance()->channels_[_name]; }

    void Initialize();
    void Update();

    double GetDeltaTime() const { return deltaTime_; }
    double GetFramerate() const { return framerate_; }

    void BeginFrame();
    void EndFrame();

    void CreateChannel(const std::string& _name);
    void EraseChannel(const std::string& _name);

    static void SetDefaultFramerate(float _framerate);
    static void SetDeltaTimeFixed(bool _isFixed);

    static float GetUnScaleDeltaTime_float();
    static double GetUnScaleDeltaTime_double();

    static bool IsFixedDeltaTime();

private:

    std::map<std::string, GameTimeChannel> channels_;

    double lastFrameTime_;    // 前回フレームの時間
    double currentFrameTime_; // 現在フレームの時間

    double deltaTime_ = 0.0f;

    float updateInterval_ = 1.0f;
    float fpsAccumulator_ = 0.0f; // フレームレートの更新頻度
    double framerate_ = 0.0f;
    uint32_t frameCount_ = 0;

private:
    GameTime();
    ~GameTime() = default;
public:
    GameTime(const GameTime&) = delete;
    GameTime& operator=(const GameTime&) = delete;


};

} // namespace Engine
