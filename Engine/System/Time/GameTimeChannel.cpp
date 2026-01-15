#include <System/Time/GameTimeChannel.h>


namespace Engine {

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

void GameTimeChannel::Update(double _deltaTime)
{
    if (hitStopTime_ > 0.0f)
    {
        // ヒットストップ中
        // カウントダウン
        hitStopTime_ -= static_cast<float>(_deltaTime);

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
        deltaTime_ = _deltaTime * gameSpeed_;
    }
}

} // namespace Engine
