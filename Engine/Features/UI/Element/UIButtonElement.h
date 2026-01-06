#pragma once

#include "UIElement.h"
#include "../Component/UINavigationComponent.h"
#include <Math/Vector/Vector4.h>
#include <functional>
#include <string>

// 前方宣言
class UISpriteRenderComponent;
class UITextRenderComponent;
class UIButtonComponent;

enum class NavigationDirection;

/// <summary>
/// テキストのアラインメント
/// </summary>
enum class TextAlignment
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/// <summary>
/// UIElement + UIComponent アーキテクチャで実装されたボタンUI
/// </summary>
class UIButtonElement : public UIElement
{
public:
    UIButtonElement(const std::string& name,
                    const Vector2& pos,
                    const Vector2& size,
                    const std::string& text = "Button",
                    bool child = false);
    ~UIButtonElement() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // コールバック設定
    void SetOnClick(std::function<void()> callback);
    void SetOnHover(std::function<void()> callback);

    void SetOnClickDown(std::function<void()> callback);
    void SetOnClickUp(std::function<void()> callback);
    void SetOnHoverEnter(std::function<void()> callback);
    void SetOnHoverExit(std::function<void()> callback);
    void SetOnHovering(std::function<void()> callback);

    void SetOnFocusEnter(std::function<void()> callback);
    void SetOnFocusExit(std::function<void()> callback);

    // テキスト設定
    void SetText(const std::string& text);
    std::string GetText() const { return text_; }

    // 色設定
    void SetNormalColor(const Vector4& color) { normalColor_ = color; }
    void SetHoverColor(const Vector4& color) { hoverColor_ = color; }
    void SetPressedColor(const Vector4& color) { pressedColor_ = color; }
    void SetDisabledColor(const Vector4& color) { disabledColor_ = color; }

    // テキスト色設定
    void SetTextColor(const Vector4& color);

    // テキストアラインメント設定
    void SetTextAlignment(TextAlignment alignment);
    TextAlignment GetTextAlignment() const { return textAlignment_; }

    // 有効/無効
    void SetButtonEnabled(bool enabled);

    // ナビゲーション設定
    void SetNavigation(NavigationDirection dir, UIElement* target);
    void SetFocusable(bool focusable);
    bool IsFocused() const;

    void DrawImGuiInspector() override;

private:
    // Component参照
    UISpriteRenderComponent* background_ = nullptr;
    UITextRenderComponent* textComponent_ = nullptr;
    UIButtonComponent* button_ = nullptr;
    UINavigationComponent* navigation_ = nullptr;

    // テキスト
    std::string text_;

    // 色設定
    Vector4 normalColor_ = { 0.8f, 0.8f, 0.8f, 1.0f };
    Vector4 hoverColor_ = { 0.9f, 0.9f, 0.9f, 1.0f };
    Vector4 pressedColor_ = { 0.6f, 0.6f, 0.6f, 1.0f };
    Vector4 disabledColor_ = { 0.5f, 0.5f, 0.5f, 0.5f };
    Vector4 focusColor_ = { 1.0f, 1.0f, 0.6f, 1.0f };  // フォーカス時の色（黄色っぽい）

    Vector4 textColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };

    // テキストアラインメント
    TextAlignment textAlignment_ = TextAlignment::Center;

    // 状態に応じた色変更
    void OnStateChanged();

    // テキスト位置計算ヘルパー
    Vector2 CalculateTextPosition() const;
    Vector2 CalculateTextPivot() const;
};
