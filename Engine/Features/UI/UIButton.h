#pragma once

#include "UISelectable.h"

enum class PadButton;

class UIButton : public UISelectable
{
public:

    UIButton();
    ~UIButton() = default;

    // 初期化
    void Initialize(const std::string& _label) override;
    void Initialize(const std::string& _label, const std::wstring& _text) override;
    void Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config) override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 入力処理
    bool HandleInput() override;

    // マウスクリックで押されたか
    bool IsPressed();
    // 指定のpadボタンで押されたか
    bool IsPressed(PadButton _button);
    // 押された コールバックを開始する
    void Pressed();

    // 色を設定する
    void SetColor(const Vector4& _color) override;

    // クリック関連のコールバックを設定する
    void SetCallBackOnClickStart(std::function<void()> _callback) { onClickStart_ = _callback; }
    void SetCallBackOnClickEnd(std::function<void()> _callback) { onClickEnd_ = _callback; }

    // 全てのコールバックを一括設定
    void SetCallBacks(std::function<void()> _onFocusGained,
        std::function<void()> _onFocusLost,
        std::function<void()> _onFocusUpdate,
        std::function<void()> _onClickStart,
        std::function<void()> _onClickEnd);

protected:

    // デフォルトコールバック
    virtual void OnClickStart();
    virtual void OnClickEnd();

private:

    // クリック状態
    bool isTriggered_ = false;
    bool isClickEnd_ = false;

    // クリック関連コールバック
    std::function<void()> onClickStart_ = nullptr;
    std::function<void()> onClickEnd_ = nullptr;
};