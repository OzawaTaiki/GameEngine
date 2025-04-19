#include "Stopwatch.h"

Stopwatch::Stopwatch(bool _useGameTime, const std::string& _channelName) :
    useGameTime_(_useGameTime),
    channelName_(_channelName),
    elapsedTime_(0.0),
    isRunning_(false)
{
}

void Stopwatch::Start()
{
    isRunning_ = true;
    elapsedTime_ = 0.0;
}

void Stopwatch::Stop()
{
    isRunning_ = false;
}

void Stopwatch::Resume()
{
    isRunning_ = true;
}

void Stopwatch::Reset()
{
    elapsedTime_ = 0.0;
}

void Stopwatch::Update()
{
    if (!isRunning_)
        return;

    if (useGameTime_)
    {
        // 指定されたチャンネルのデルタタイムを加算
        elapsedTime_ += GameTime::GetChannel(channelName_).GetDeltaTime<double>();
    }
    else
    {
        elapsedTime_ += Time::GetDeltaTime<double>();
    }
}

