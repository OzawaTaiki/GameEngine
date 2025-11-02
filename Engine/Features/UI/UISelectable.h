#pragma once

#include <Features/UI/UIInteractive.h>

#include <vector>

enum class Direction
{
    Up,
    Down,
    Left,
    Right,
    None
};

// 選択可能なUI基底クラス
class UISelectable : public UIInteractive
{
public:
    UISelectable();
    virtual ~UISelectable() = default;

    void Initialize(const std::string& _label, bool _regsterDebugWindow = true) override;

    void UpdateSelf () override;


    // 選択状態
    void SetSelected(bool _selected) { isSelected_ = _selected; }
    bool IsSelected() const { return isSelected_; }

    // フォーカス状態
    void SetFocused(bool _focused);
    bool IsFocused() const { return isFocused_; }

    // ナビゲーション
    void SetNavigationTarget(UISelectable* _target, Direction _dir);
    UISelectable* GetNavigationTarget(Direction _dir) const;

    // ドラッグ関連
    bool IsDragging() const { return isDragging_; }
    // ドラッグのコールバック設定
    void SetOnDragStart (std::function<void()> _callback) { onDragStartCallback_ = _callback; }
    void SetOnDragging  (std::function<void()> _callback) { onDraggingCallback_ = _callback; }
    void SetOnDragEnd   (std::function<void()> _callback) { onDragEndCallback_ = _callback; }

    // 色設定
    void SetDefaultColor    (const Vector4& _color) { defaultColor_ = _color; }
    void SetHoverColor      (const Vector4& _color) { hoverColor_ = _color; }
    void SetPressedColor    (const Vector4& _color) { pressedColor_ = _color; }
    void SetSelectedColor   (const Vector4& _color) { selectedColor_ = _color; }
    void SetFocusedColor    (const Vector4& _color) { focusedColor_ = _color; }

    void ImGuiContent() override;
protected:

    // ドラッグイベント
    virtual void OnDragStart();
    virtual void OnDragging();
    virtual void OnDragEnd();

    // フォーカスイベント
    virtual void OnFocusGained();
    virtual void OnFocusLost();

    // 現在の色を取得
    Vector4 GetCurrentColor() const;

    // ナビゲーション処理
    void ProcessNavigation();

protected:

    // フォーカス関連
    bool isSelected_ = false; // 選択状態か
    bool isFocused_  = false; // フォーカス状態か

    // ドラッグ状態
    bool    isDragging_     = false; // ドラッグ中か
    Vector2 dragStartPos_   = { 0,0 }; // ドラッグ開始時のマウス位置
    Vector2 dragOffset_     = { 0,0 }; // ドラッグ中のオフセット


    // ナビゲーション関連
    UISelectable* upTarget_     = nullptr;
    UISelectable* downTarget_   = nullptr;
    UISelectable* leftTarget_   = nullptr;
    UISelectable* rightTarget_  = nullptr;

    // コールバック
    std::function<void()> onDragStartCallback_  = nullptr;
    std::function<void()> onDraggingCallback_   = nullptr;
    std::function<void()> onDragEndCallback_    = nullptr;

    // 状態別カラー
    Vector4 defaultColor_   = { 1.0f,1.0f,1.0f,1.0f }; // デフォルトカラー
    Vector4 hoverColor_     = { 0.9f,0.9f,0.9f,1.0f }; // ホバーカラー
    Vector4 pressedColor_   = { 0.7f,0.7f,0.7f,1.0f }; // 押下カラー
    Vector4 selectedColor_  = { 0.8f,0.8f,1.0f,1.0f }; // 選択カラー
    Vector4 focusedColor_   = { 1.0f,1.0f,0.8f,1.0f }; // フォーカスカラー

};