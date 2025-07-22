#pragma once

#include "UIBase.h"

enum class PadButton;

enum class Direction
{
    Up,
    Down,
    Left,
    Right,

    None
};


class UIButton :public UIBase
{
public:

    UIButton();
    ~UIButton() = default;

    // 初期化
    void Initialize(const std::string& _label);
    // 初期化
    void Initialize(const std::string& _label, const std::wstring& _text) override;
    // 更新
    void Update() override;
    // 描画
    void Draw() override;

    // マウス右クリックで押されたか
    bool IsPressed();
    // 指定のpadボタンで押されたか
    bool IsPressed(PadButton _button);
    // 押された コールバックを開始する
    void Pressed();

    // サイズを取得する
    Vector2 GetSize() const { return size_; }

    // 色を設定する
    void SetColor(const Vector4& _color) override;

    // ナビゲーションのターゲットを設定する
    void SetNavigationTarget(UIButton* _target, Direction _dir);

    // ナビゲーションのターゲットを取得する
    UIButton* GetNavigationTarget(Direction _dir) const;

    // フォーカスされているかどうか設定する
    void SetFocused(bool _isFocused);


    // コールバックを設定する
    void SetCallBacks(std::function<void(void)> _onFocusGained,
        std::function<void(void)> _onFocusLost,
        std::function<void(void)> _onFocusUpdate,
        std::function<void(void)> _onClickStart,
        std::function<void(void)> _onClickEnd);

    // コールバックを設定する
    void SetCallBackOnFocusGained(std::function<void(void)> _onFocusGained) { onFocusGained_ = _onFocusGained; }
    // コールバックを設定する
    void SetCallBackOnFocusLost(std::function<void(void)> _onFocusLost) { onFocusLost_ = _onFocusLost; }
    // コールバックを設定する
    void SetCallBackOnFocusUpdate(std::function<void(void)> _onFocusUpdate) { onFocusUpdate_ = _onFocusUpdate; }
    // コールバックを設定する
    void SetCallBackOnClickStart(std::function<void(void)> _onClickStart) { onClickStart_ = _onClickStart; }
    // コールバックを設定する
    void SetCallBackOnClickEnd(std::function<void(void)> _onClickEnd) { onClickEnd_ = _onClickEnd; }

private:

    // デフォルトコールバック
    void OnFocusGained();
    void OnFocusLost();
    void OnClickStart();
    void OnClickEnd();
private:

    // ナビゲーションのターゲット
    UIButton* upButton_ = nullptr; // 上のボタン
    UIButton* downButton_ = nullptr; // 下のボタン
    UIButton* leftButton_ = nullptr; // 左のボタン
    UIButton* rightButton_ = nullptr; // 右のボタン


    // コールバック関数たち
    std::function<void(void)> onFocusGained_ = nullptr; // フォーカスを得たときのコールバック
    std::function<void(void)> onFocusLost_ = nullptr; // フォーカスを失ったときのコールバック
    std::function<void(void)> onFocusUpdate_ = nullptr; // フォーカス更新時のコールバック, 例えば色を変えるなどの処理を行うためのもの
    std::function<void(void)> onClickStart_ = nullptr; // クリック時のコールバック SEや視覚効果など
    std::function<void(void)> onClickEnd_ = nullptr; // クリック処理終了時のコールバック


    // クリック後の処理が終了したか
    bool isClickEnd_ = false;

    // クリックが開始されたか
    bool isTrigered_ = false;

    //フォーカスされているか
    bool isFocused_ = false;

    // 変化前の色
    Vector4 defaultColor = { 1,1,1,1 };
};


