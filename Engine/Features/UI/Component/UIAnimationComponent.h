#pragma once

#include <Features/UI/Component/UIComponent.h>

#include <Features/Animation/Sequence/AnimationSequence.h>

namespace Engine
{

class UIAnimationComponent : public UIComponent
{
public:
    UIAnimationComponent(UIElement* owner,const std::string& animName);
    ~UIAnimationComponent() = default;

    void Initialize() override;
    void Update() override;

    // 描画は不要
    // void Draw() override;

    void DrawImGui() override;

    // アニメーション再生
    // アニメーションの始めから再生する
    void Play();

    // アニメーション一時停止
    // 再生位置は維持される
    void Pause();

    // アニメーション停止
    // 再生位置はリセットされる
    void Stop();

    bool IsPlaying() const { return isPlaying_; }
    bool IsEnded() const;

    void SetOnAnimationEndCallback(const std::function<void()>& callback) { onAnimationEnd_ = callback; }

    //void SetAnimationName(const std::string& animName) { animationName_ = animName; }
    void SetUseTimeChannel(const std::string& channelName) { useTimeChannel_ = channelName; }

private:
    struct BaseTransform
    {
        Vector2 position;
        Vector2 size;
        Vector2 scale;
        float   rotation;
    };
    BaseTransform baseTransform_;  // Play()時に記録

private:

    std::function<void()> onAnimationEnd_ = nullptr;

    std::unique_ptr<AnimationSequence> animationSequence_;

    std::string useTimeChannel_;
    std::string animationName_;

    bool isPlaying_ = false;

#ifdef _DEBUG
    bool showSeq = true;
#endif

};

}// namespace Engine