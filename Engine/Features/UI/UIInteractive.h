#pragma once

#include <Features/UI/UIBase.h>
#include <functional>

// 入力機能を持つUI基底クラス
class UIInteractive : public UIBase
{
public:
    UIInteractive() = default;
    virtual ~UIInteractive() = default;

    void UpdateSelf() override; // 自身の更新処理

    // マウス判定
    bool IsMousePointerInside() const;
    bool IsHovered() const { return isHovered_; }
    bool IsPressed() const { return isPressed_; }


    // イベントコールバック
    void SetOnClick     (std::function<void()> _callback) { onClickCallback_ = _callback; }
    void SetOnHoverEnter(std::function<void()> _callback) { onHoverEnterCallback_ = _callback; }
    void SetOnHovering  (std::function<void()> _callback) { onHoveringCallback_ = _callback; }
    void SetOnHoverExit (std::function<void()> _callback) { onHoverExitCallback_ = _callback; }


    void ImGuiContent() override;
protected:

    virtual void OnClick();         // クリック時の処理（オーバーライド可能）
    virtual void OnHoverEnter();    // ホバー開始時の処理（オーバーライド可能）
    virtual void OnHovering();      // ホバー中の処理（オーバーライド可能）
    virtual void OnHoverExit();     // ホバー終了時の処理（オーバーライド可能)
    virtual void OnMouseDown();     // マウスダウン時の処理（オーバーライド可能）
    virtual void OnMouseUp();       // マウスアップ時の処理（オーバーライド可能）



protected:

    bool isHovered_     = false; // マウスオーバーしているか
    bool wasHovered_    = false; // 前フレームでマウスオーバーしていたか
    bool isPressed_     = false; // マウスボタンが押されているか

    std::function<void()> onClickCallback_      = nullptr;
    std::function<void()> onHoverEnterCallback_ = nullptr;
    std::function<void()> onHoveringCallback_   = nullptr;
    std::function<void()> onHoverExitCallback_  = nullptr;
};