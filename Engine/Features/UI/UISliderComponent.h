#pragma once

#include "UIComponent.h"
#include <Math/Vector/Vector2.h>

#include <functional>

// 前方宣言
class UIElement;

/// <summary>
/// スライダーの値管理とインタラクション処理を行うコンポーネント
/// </summary>
class UISliderComponent : public UIComponent
{
public:
    UISliderComponent() = default;
    ~UISliderComponent() override = default;

    void Initialize() override;
    void Update() override;
    void DrawImGui() override;

    // トラックとハンドルの設定
    void SetTrack(UIElement* track) { track_ = track; }
    void SetHandle(UIElement* handle) { handle_ = handle; }

    // 値の設定/取得
    void SetValue(float value);
    float GetValue() const { return value_; }

    // 範囲設定
    void SetRange(float min, float max);
    float GetMinValue() const { return minValue_; }
    float GetMaxValue() const { return maxValue_; }

    // コールバック設定
    void SetOnValueChanged(std::function<void(float)> callback) { onValueChanged_ = callback; }

    // 有効/無効
    void SetEnabled(bool enabled) { isEnabled_ = enabled; }
    bool IsEnabled() const { return isEnabled_; }

private:
    UIElement* track_ = nullptr;
    UIElement* handle_ = nullptr;

    float value_ = 0.5f;
    float minValue_ = 0.0f;
    float maxValue_ = 1.0f;

    bool isDragging_ = false;
    bool isEnabled_ = true;

    std::function<void(float)> onValueChanged_;

    // 内部処理
    void UpdateValueFromMousePosition(const Vector2& mousePos);
    void UpdateHandlePosition();
    bool IsMouseOverHandle(const Vector2& mousePos) const;
    bool IsMouseOverTrack(const Vector2& mousePos) const;
};
