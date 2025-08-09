#pragma once

#include <Features/UI/UIBase.h>
#include <functional>

enum class Direction
{
    Up,
    Down,
    Left,
    Right,
    None
};

class UISelectable : public UIBase
{
public:

    UISelectable();
    virtual ~UISelectable() = default;

    // 初期化
    void Initialize(const std::string& _label) override;
    void Initialize(const std::string& _label, const std::wstring& _text) override;
    void Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config) override;

    // 更新
    void Update() override;

    // フォーカス関連
    virtual void SetFocused(bool _focused);
    bool IsFocused() const { return isFocused_; }
    bool IsFocusable() const { return isFocusable_; }
    void SetFocusable(bool _focusable) { isFocusable_ = _focusable; }

    // ナビゲーション関連
    void SetNavigationTarget(UISelectable* _target, Direction _dir);
    UISelectable* GetNavigationTarget(Direction _dir) const;

    // コールバック設定
    void SetCallBackOnFocusGained(std::function<void()> _callback) { onFocusGained_ = _callback; }
    void SetCallBackOnFocusLost(std::function<void()> _callback) { onFocusLost_ = _callback; }
    void SetCallBackOnFocusUpdate(std::function<void()> _callback) { onFocusUpdate_ = _callback; }

    // 入力処理（派生クラスでオーバーライド）
    virtual bool HandleInput() { return IsConfirmed(); }
    void SetConfirmKeys(const std::vector<int32_t>& _keys) { confirmKeys_ = _keys; }
    void SetConfirmKey(int32_t _key) { confirmKeys_.push_back(_key); }

protected:

    bool IsConfirmed() const;

    // デフォルトコールバック（派生クラスでオーバーライド可能）
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnFocusUpdate();

protected:

    // フォーカス状態
    bool isFocusable_ = true;
    bool isFocused_ = false;

    // ナビゲーションターゲット
    UISelectable* upTarget_ = nullptr;
    UISelectable* downTarget_ = nullptr;
    UISelectable* leftTarget_ = nullptr;
    UISelectable* rightTarget_ = nullptr;

    // コールバック関数
    std::function<void()> onFocusGained_ = nullptr;
    std::function<void()> onFocusLost_ = nullptr;
    std::function<void()> onFocusUpdate_ = nullptr;

    std::vector<int32_t> confirmKeys_ = {};

    // デフォルト色（フォーカス時の色変更用）
    Vector4 defaultColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
};