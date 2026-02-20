#include <System/Time/GameTime.h>
#include <System/Time/Time_MT.h>


namespace Engine {

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

void GameTime::BeginFrame()
{
    // 現在のフレーム時間を取得
    currentFrameTime_ = Time_MT::GetTotalTime();


    // 前回フレームからの経過時間を計算
    double delt = currentFrameTime_ - lastFrameTime_;

    if (delt < 0.0f)
        delt = 1.0f / 60.0f;

    deltaTime_ = delt;

    frameCount_++;
    fpsAccumulator_ += static_cast<float>(deltaTime_);

    if(fpsAccumulator_>=updateInterval_)
    {
        framerate_ = static_cast<double>(frameCount_) / fpsAccumulator_;
        fpsAccumulator_ = 0.0f;
        frameCount_ = 0;
    }


    // 各チャンネルにデルタタイムを設定
    for (auto& channel : channels_)
    {
        channel.second.Update(deltaTime_);
    }

    lastFrameTime_ = currentFrameTime_;
}

void GameTime::EndFrame()
{
    lastFrameTime_ = currentFrameTime_;
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
    Time_MT::SetDefaultFramerate(_framerate);
}

void GameTime::SetDeltaTimeFixed(bool _isFixed)
{
    Time_MT::SetDeltaTimeFixed(_isFixed);
}

float GameTime::GetUnScaleDeltaTime_float()
{
    return Time_MT::GetDeltaTime<float>();
}

double GameTime::GetUnScaleDeltaTime_double()
{
    return Time_MT::GetDeltaTime<double>();
}

bool GameTime::IsFixedDeltaTime()
{
    return Time_MT::IsDeltaTimeFixed();
}

} // namespace Engine
