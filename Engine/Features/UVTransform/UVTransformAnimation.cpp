#include <Features/UVTransform/UVTransformAnimation.h>

#include <Features/UVTransform/UVTransform.h>

UVTransformAnimation::UVTransformAnimation():
    transforms_(),
    duration_(1.0f),
    currentTime_(0.0f),
    isPlaying_(false),
    isLooping_(false),
    onComplete_(nullptr),
    scrollSpeed_(),
    scaleSpeed_(),
    rotationSpeed_(0.0f)
{
}

void UVTransformAnimation::AddTransform(UVTransform* _transform)
{
    if(_transform)
    {
        transforms_.push_back(_transform);
    }
}

void UVTransformAnimation::RemoveTransform(UVTransform* _transform)
{
    auto itr = std::find(transforms_.begin(), transforms_.end(), _transform);
    if (itr != transforms_.end())
    {
        transforms_.erase(itr);
    }
}

void UVTransformAnimation::Update(float _deltaTime)
{
    if (!isPlaying_) return;

    currentTime_ += _deltaTime;
    if (currentTime_ >= duration_) {
        if (isLooping_) {
            currentTime_ = std::fmod(currentTime_, duration_);
        }
        else {
            currentTime_ = duration_;
            isPlaying_ = false;
            if (onComplete_) {
                onComplete_();
            }
            return;
        }
    }

    for (auto* transform : transforms_) {
        if (transform) {
            // スクロール更新
            Vector2 currentOffset = transform->GetOffset();
            transform->SetOffset(
                currentOffset.x + scrollSpeed_.x * _deltaTime,
                currentOffset.y + scrollSpeed_.y * _deltaTime
            );

            // 回転更新
            float currentRotation = transform->GetRotation();
            transform->SetRotation(currentRotation + rotationSpeed_ * _deltaTime);

            // スケール更新
            Vector2 currentScale = transform->GetScale();
            transform->SetScale(
                currentScale.x + scaleSpeed_.x * _deltaTime,
                currentScale.y + scaleSpeed_.y * _deltaTime
            );
        }
    }
}

void UVTransformAnimation::Play()
{
    isPlaying_ = true;
}

void UVTransformAnimation::Stop()
{
    isPlaying_ = false;
}

void UVTransformAnimation::Reset()
{
    currentTime_ = 0.0f;
    isPlaying_ = false;
    for (auto* transform : transforms_) {
        if (transform) {
            transform->SetOffset(0.0f, 0.0f);
            transform->SetRotation(0.0f);
            transform->SetScale(1.0f, 1.0f);
        }
    }
}

void UVTransformAnimation::SetDuration(float duration)
{
    duration_ = duration;
}

void UVTransformAnimation::SetLooping(bool looping)
{
    isLooping_ = looping;
}

void UVTransformAnimation::SetOnComplete(std::function<void()> _callback)
{
    onComplete_ = _callback;
}

bool UVTransformAnimation::IsPlaying() const
{
    return isPlaying_;
}

float UVTransformAnimation::GetProgress() const
{
    return currentTime_ / duration_;
}
