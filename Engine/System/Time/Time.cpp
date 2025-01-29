#include <System/Time/Time.h>
#include <chrono>

double  Time::deltaTime_                = 1.0f / 60.0f;
double  Time::totalTime_                = 0.0f;
double  Time::prevTime_                 = 0.0f;
double  Time::currentTime_              = 0.0f;
double  Time::framerate_                = 60.0f;
double  Time::updateInterval_           = 1.0f;
float   Time::defaultFramerate_         = 60.0f;
bool    Time::isDeltaTimeFixed_         = true;
int32_t Time::frameCount_               = 0;

void Time::Initialize()
{
    // メンバ変数の初期化
    deltaTime_ = 1.0f / 60.0f;
    totalTime_ = 0.0f;
    prevTime_ = 0.0f;
    currentTime_ = 0.0f;
    framerate_ = 60.0f;
    updateInterval_ = 1.0f;
    defaultFramerate_ = 60.0f;
    isDeltaTimeFixed_ = true;
    frameCount_ = 0;
}

void Time::Update()
{
    // 現在時間を取得
    currentTime_ = GetCurrentTime();

    // 更新間隔をチェック
    if (currentTime_ - prevTime_ <= updateInterval_)
    {
        ++frameCount_;
        return;
    }

    // デルタタイムを更新
    if (isDeltaTimeFixed_)
        deltaTime_ = 1.0f / defaultFramerate_;
    else
        deltaTime_ = (currentTime_ - prevTime_) / static_cast<double>(frameCount_);

    // 時間の更新
    prevTime_ = currentTime_;
    frameCount_ = 0;

    // デルタタイムの異常値チェック
    if (deltaTime_ <= 0.0f) {
        deltaTime_ = 1.0f / defaultFramerate_;
    }

    // 総時間とデルタタイムの更新
    totalTime_ += deltaTime_;
    framerate_ = 1.0f / deltaTime_;
}

double Time::GetCurrentTime()
{
    // ミリ秒単位で現在時間を取得
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

    // 秒単位に変換して返す
    return static_cast<float>(duration.count()) / 1000.0f;
}
