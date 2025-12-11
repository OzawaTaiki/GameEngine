#pragma once

#include "UIElement.h"
#include <Math/Vector/Vector4.h>
#include <functional>
#include <string>

// 前方宣言
class UISpriteRenderComponent;
class UISliderComponent;

/// <summary>
/// UIElement + UIComponent アーキテクチャで実装されたスライダーUI
/// </summary>
class UISliderElement : public UIElement
{
public:
    UISliderElement(const std::string& name,
                    const Vector2& pos,
                    const Vector2& size,
                    bool child = false);
    ~UISliderElement() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // 値の設定/取得
    void SetValue(float value);
    float GetValue() const;

    // 範囲設定
    void SetRange(float min, float max);

    // コールバック設定
    void SetOnValueChanged(std::function<void(float)> callback);

    // 色設定
    void SetTrackColor(const Vector4& color);
    void SetHandleColor(const Vector4& color);
    void SetHandleHoverColor(const Vector4& color);

    // 有効/無効
    void SetSliderEnabled(bool enabled);

    void DrawImGuiTree() override;
    void DrawImGuiInspector() override;

private:
    // 子要素
    UIElement* track_ = nullptr;
    UIElement* handle_ = nullptr;

    // Component参照
    UISpriteRenderComponent* trackSprite_ = nullptr;
    UISpriteRenderComponent* handleSprite_ = nullptr;
    UISliderComponent* slider_ = nullptr;

    // 色設定
    Vector4 trackColor_ = { 0.3f, 0.3f, 0.3f, 1.0f };
    Vector4 handleColor_ = { 0.8f, 0.8f, 0.8f, 1.0f };
    Vector4 handleHoverColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };

    // サイズ設定
    float handleWidth_ = 20.0f;
    float handleHeight_ = 30.0f;

    // 状態に応じた色変更
    void OnStateChanged();
};
