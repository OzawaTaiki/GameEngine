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

    void SetOnClick(std::function<void()> callback) { onClick_ = callback; }
    void SetOnHover(std::function<void()> callback) { onHover_ = callback; }

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

    std::function<void()> onClick_;
    std::function<void()> onHover_;

};
