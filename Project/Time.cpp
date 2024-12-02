#include "Time.h"
#include <chrono>

double Time::deltaTime_ = 1.0f / 60.0f;
double Time::totalTime_ = 0.0f;
double Time::prevTime_ = 0.0f;
double Time::currentTime_ = 0.0f;
double Time::framerate_ = 60.0f;

double Time::deltaTimeUpdateInterval_ = 1.0f;
int32_t Time::frameCount_ = 0;


void Time::Initialize()
{
    deltaTime_ = 1.0f / 60.0f;
    totalTime_ = 0.0f;
    prevTime_ = GetCurrentTime();
    currentTime_ = prevTime_;
    framerate_ = 60.0f;
}

void Time::Update()
{
    currentTime_ = GetCurrentTime();

    if (currentTime_ - prevTime_ <= deltaTimeUpdateInterval_)
    {
        ++frameCount_;
        return;
    }

    deltaTime_ = (currentTime_ - prevTime_) /static_cast<double>(frameCount_);
    prevTime_ = currentTime_;
    frameCount_ = 0;

    if (deltaTime_ <= 0.0f) {
        deltaTime_ = 1.0f / 60.0f;
    }

    totalTime_ += deltaTime_;
    framerate_ = 1.0f / deltaTime_;
}

double Time::GetCurrentTime()
{
    // ミリ秒単位の現在時間を秒単位で返す
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<float>(duration.count()) / 1000.0f;
}
