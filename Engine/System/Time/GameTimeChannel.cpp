#include <System/Time/GameTimeChannel.h>

GameTimeChannel::GameTimeChannel():
    deltaTime_(1.0f / 60.0f),
    gameSpeed_(1.0f),
    isPaused_(false),
    hitStopTime_(0.0f)
{
}

void GameTimeChannel::Initialize()
{
}

void GameTimeChannel::Update()
{
    if (hitStopTime_ > 0.0f)
    {
        // ヒットストップ中
        // カウントダウン
        hitStopTime_ -= Time::GetDeltaTime<float>();

        if (hitStopTime_ <= 0.0f)
            hitStopTime_ = 0.0f;

        // 時間を止める
        deltaTime_ = 0.0f;
    }
    else if (isPaused_)
    {
        // ポーズ中 時間を止める
        deltaTime_ = 0.0f;
    }
    else
    {
        // 通常時
        deltaTime_ = Time::GetDeltaTime<double>() * gameSpeed_;
    }
}
