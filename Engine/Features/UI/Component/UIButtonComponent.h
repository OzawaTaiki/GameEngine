#pragma once

#include "UIComponent.h"
#include <functional>

enum class ButtonState
{
    Normal,
    Hovered,
    Pressed,
    Disabled
};

class UIColliderComponent;

class UIButtonComponent : public UIComponent
{
public:
    UIButtonComponent() = default;
    ~UIButtonComponent() override = default;

    void Initialize() override;
    void Update() override;
    void DrawImGui() override;

    void SetOnClick(std::function<void()> callback)         { onClick_ = callback; }
    void SetOnHover(std::function<void()> callback)         { onHover_ = callback; }

    void SetOnClickDown(std::function<void()> callback)     { onClickDown_ = callback; }
    void SetOnClickUp(std::function<void()> callback)       { onClickUp_ = callback; }
    void SetOnHoverEnter(std::function<void()> callback)    { onHoverEnter_ = callback; }
    void SetOnHoverExit(std::function<void()> callback)     { onHoverExit_ = callback; }
    void SetOnHovering(std::function<void()> callback)      { onHovering_ = callback; }

    ButtonState GetState() const { return state_; }
    void SetEnabled(bool enabled);

    // キーボード/ゲームパッドからの実行
    void Execute();

private:

    void UpdateState();

private:

    UIColliderComponent* collider_ = nullptr;
    ButtonState state_ = ButtonState::Normal;

    bool isEnabled_ = true;

    std::function<void()> onHoverEnter_;  // マウスが要素に入った瞬間
    std::function<void()> onHovering_;    // マウスが要素の上にある間（毎フレーム）
    std::function<void()> onHoverExit_;   // マウスが要素から出た瞬間
    std::function<void()> onClickDown_;   // マウスボタンが押された瞬間
    std::function<void()> onClickUp_;     // マウスボタンが離された瞬間

    std::function<void()> onClick_;
    std::function<void()> onHover_;

};
