#pragma once

#include <Features/UI/UIButton.h>

enum class UIAction
{
    NavigateUp,
    NavigateDown,
    NavigateLeft,
    NavigateRight,
    Confirm,    // 決定

    None
};

class Input;

class ButtonNavigator
{
public:

    ButtonNavigator() = default;
    ~ButtonNavigator() = default;

    void Initialize(UIButton* _focusedButton);

    void Update();

    void SetFocusedButton(UIButton* _button) { currentFocusedButton_ = _button; }

    void RegisterButton(UIButton* _button);

public:

    static void SetKeyConfig(UIAction _action, std::vector<int32_t> _keyCodes) { keyConfing[_action] = std::move(_keyCodes); }

private:

    void CheckButtonFocus();

    Direction GetDirectionFromInput() const;

    bool CheckConfirm() const;

    void ChangeFocus(UIButton* _button);

private:

    using KeyConfig = std::map<UIAction, std::vector<int32_t>>;
    static KeyConfig keyConfing;


private:
    Input* input_ = nullptr;

    // 現在のフォーカスを持つボタン
    UIButton* currentFocusedButton_ = nullptr;

    std::vector<UIButton*> registeredButtons_; // 登録されたボタンのリスト

    Vector2 preMousePosition_ = { 0.0f, 0.0f }; // 前回のマウス位置
};