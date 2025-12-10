#pragma once

#include "UIComponent.h"
#include <memory>

// 前方宣言
class UIElement;

/// <summary>
/// ナビゲーション方向
/// </summary>
enum class NavigationDirection
{
    Up,
    Down,
    Left,
    Right
};

/// <summary>
/// キーボード/ゲームパッドでのUI要素間のナビゲーションを提供するコンポーネント
/// </summary>
class UINavigationComponent : public UIComponent
{
public:
    UINavigationComponent() = default;
    ~UINavigationComponent() override = default;

    void Initialize() override;
    void Update() override;
    void DrawImGui() override;

    // Navigation設定
    void SetNavigation(NavigationDirection dir, UIElement* target);
    UIElement* GetNext(NavigationDirection dir) const;

    // Focusable設定
    void SetFocusable(bool focusable) { focusable_ = focusable; }
    bool IsFocusable() const { return focusable_; }

    // フォーカス状態
    void SetFocused(bool focused) { isFocused_ = focused; }
    bool IsFocused() const { return isFocused_; }

private:
    // 4方向のNavigation先（weak_ptrで循環参照を防止）
    UIElement* up_ = nullptr;
    UIElement* down_ = nullptr;
    UIElement* left_ = nullptr;
    UIElement* right_ = nullptr;

    bool focusable_ = true;
    bool isFocused_ = false;
};
