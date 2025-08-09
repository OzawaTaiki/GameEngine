#pragma once

#include <Features/UI/UISelectable.h>
#include <map>
#include <vector>

enum class UIAction
{
    NavigateUp,
    NavigateDown,
    NavigateLeft,
    NavigateRight,
    Confirm,

    None
};

class Input;

class UINavigator
{
public:

    UINavigator() = default;
    ~UINavigator() = default;

    void Initialize(UISelectable* _focusedElement = nullptr);

    void Update();

    void SetFocused(UISelectable* _element) { currentFocused_ = _element; }
    UISelectable* GetFocused() const { return currentFocused_; }

    void RegisterSelectable(UISelectable* _element);
    void UnregisterSelectable(UISelectable* _element);
    void ClearSelectables();

    // キーコンフィグ設定
    static void SetKeyConfig(UIAction _action, std::vector<int32_t> _keyCodes) { keyConfig_[_action] = std::move(_keyCodes); }

private:

    void CheckElementFocus();

    Direction GetDirectionFromInput() const;

    bool CheckConfirm() const;

    void ChangeFocus(UISelectable* _element);

private:

    using KeyConfig = std::map<UIAction, std::vector<int32_t>>;
    static KeyConfig keyConfig_;

private:
    Input* input_ = nullptr;

    // 現在のフォーカスを持つ要素
    UISelectable* currentFocused_ = nullptr;

    std::vector<UISelectable*> registeredElements_; // 登録された要素のリスト

    Vector2 previousMousePosition_ = { 0.0f, 0.0f }; // 前回のマウス位置
};