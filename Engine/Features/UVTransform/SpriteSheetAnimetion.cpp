#include <Features/UVTransform/SpriteSheetAnimetion.h>

#include <Features/UVTransform/UVTransform.h>

SpriteSheetAnimation::SpriteSheetAnimation() :
    transforms_(),
    duration_(1.0f),
    currentTime_(0.0f),
    isPlaying_(false),
    isLooping_(false),
    onComplete_(nullptr),
    sheetNumX_(1),
    sheetNumY_(1),
    sheetNum_(1),
    switchSpeed_(0.1f)
{
}

SpriteSheetAnimation::SpriteSheetAnimation(uint32_t _sheetNumX, uint32_t _sheetNumY, uint32_t _sheetNum, float _switchSpeed) :
    transforms_(),
    duration_(1.0f),
    currentTime_(0.0f),
    isPlaying_(false),
    isLooping_(false),
    onComplete_(nullptr),
    sheetNumX_(_sheetNumX),
    sheetNumY_(_sheetNumY),
    sheetNum_(_sheetNum),
    switchSpeed_(_switchSpeed)
{
    sheetSize_ = Vector2(1.0f / static_cast<float>(sheetNumX_), 1.0f / static_cast<float>(sheetNumY_));
}

void SpriteSheetAnimation::AddTransform(UVTransform* _transform)
{
    if (_transform)
    {
        transforms_.push_back(_transform);
    }
}

void SpriteSheetAnimation::RemoveTransform(UVTransform* _transform)
{
    auto itr = std::find(transforms_.begin(), transforms_.end(), _transform);
    if (itr != transforms_.end())
    {
        transforms_.erase(itr);
    }
}

void SpriteSheetAnimation::Update(float _deltaTime)
{
    if (!isPlaying_)
        return;

    currentTime_ += _deltaTime;
    if (currentTime_ >= duration_)
    {
        if (isLooping_)
        {
            currentTime_ = std::fmod(currentTime_, duration_);
        }
        else
        {
            // 再生時間が終了したら再生を停止
            currentTime_ = duration_;
            isPlaying_ = false;
            if (onComplete_)
            {
                // コールバック関数を呼び出す
                onComplete_();
            }
            return;
        }
    }

    for (auto* transform : transforms_)
    {
        if (transform)// NULLチェック
        {
            // 連番画像のインデックスを計算
            uint32_t nextSheetIndex = static_cast<uint32_t>(currentTime_ / switchSpeed_);
            if (currentSheetIndex_ != nextSheetIndex)
            {
                currentSheetIndex_ = nextSheetIndex;
            }

            // UV値を設定
            transform->SetOffset(
                sheetSize_.x * (currentSheetIndex_ % sheetNumX_),
                sheetSize_.y * (currentSheetIndex_ / sheetNumX_)
            );
        }
    }

}

void SpriteSheetAnimation::Play()
{
    isPlaying_ = true;
}

void SpriteSheetAnimation::Stop()
{
    isPlaying_ = false;
}

void SpriteSheetAnimation::Reset()
{
    currentTime_ = 0.0f;
    isPlaying_ = false;
}

void SpriteSheetAnimation::SetDuration(float _duration)
{
    duration_ = _duration;
}

void SpriteSheetAnimation::SetLooping(bool _looping)
{
    isLooping_ = _looping;
}

void SpriteSheetAnimation::SetOnComplete(std::function<void()> _callback)
{
    onComplete_ = _callback;
}

bool SpriteSheetAnimation::IsPlaying() const
{
    return isPlaying_;
}

float SpriteSheetAnimation::GetProgress() const
{
    return currentTime_ / duration_;
}

void SpriteSheetAnimation::SetSheetNumX(uint32_t _sheetNumX)
{
    sheetNumX_ = _sheetNumX;
    sheetSize_ = Vector2(1.0f / static_cast<float>(sheetNumX_), 1.0f / static_cast<float>(sheetNumY_));
}

void SpriteSheetAnimation::SetSheetNumY(uint32_t _sheetNumY)
{
    sheetNumY_ = _sheetNumY;
    sheetSize_ = Vector2(1.0f / static_cast<float>(sheetNumX_), 1.0f / static_cast<float>(sheetNumY_));
}

void SpriteSheetAnimation::SetSheetNum(uint32_t _sheetNum)
{
    sheetNum_ = _sheetNum;
}

void SpriteSheetAnimation::SetSwitchSpeed(float _switchSpeed)
{
    switchSpeed_ = _switchSpeed;
}
