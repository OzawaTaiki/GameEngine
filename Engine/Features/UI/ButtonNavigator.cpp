#include "ButtonNavigator.h"

#include <System/Input/Input.h>

ButtonNavigator::KeyConfig ButtonNavigator::keyConfing =
{
    {UIAction::NavigateUp,      {DIK_UP,DIK_W, static_cast<int32_t>(PadButton::iPad_Up)}},
    {UIAction::NavigateDown,    {DIK_DOWN,DIK_S, static_cast<int32_t>(PadButton::iPad_Down)}},
    {UIAction::NavigateLeft,    {DIK_LEFT,DIK_A, static_cast<int32_t>(PadButton::iPad_Left)}},
    {UIAction::NavigateRight,   {DIK_RIGHT,DIK_D, static_cast<int32_t>(PadButton::iPad_Right)}},

    {UIAction::Confirm,         {DIK_RETURN,DIK_SPACE, static_cast<int32_t>(PadButton::iPad_A)}}
};


void ButtonNavigator::Initialize(UIButton* _focusedButton)
{
    input_ = Input::GetInstance();

    currentFocusedButton_ = _focusedButton;
}

void ButtonNavigator::Update()
{
    if (!input_)
        return;

    // マウスの移動検出閾値
    const float mouseMoveThreshold = 5.0f;
    // マウスの位置を取得
    Vector2 mousePosition = input_->GetMousePosition();
    if ((mousePosition - preMousePosition_).Length() > mouseMoveThreshold)
    {
        // マウスが動いた場合,マウスによるフォーカスをチェック
        CheckButtonFocus();
    }
    if (Direction dir = GetDirectionFromInput(); dir != Direction::None)
    {
        if (currentFocusedButton_)
        {
            if (UIButton* nextFoucus = currentFocusedButton_->GetNavigationTarget(dir); nextFoucus)
            {
                ChangeFocus(nextFoucus);
            }
        }
        else
        {
            if(!registeredButtons_.empty())
            {
                ChangeFocus(registeredButtons_[0]);
            }
        }
    }

    CheckConfirm();

    preMousePosition_ = mousePosition;

}

void ButtonNavigator::RegisterButton(UIButton* _button)
{
    if (_button)
    {
        registeredButtons_.push_back(_button);
    }
}

void ButtonNavigator::CheckButtonFocus()
{

    // 判定があったか
    bool isFocused = false;
    for (auto& button : registeredButtons_)
    {
        if (button->IsMousePointerInside())
        {
            ChangeFocus(button);
            currentFocusedButton_->IsPressed();
            isFocused = true;
            break;
        }
    }

    // 判定がなかったら選択をなくす
    if (!isFocused)
    {
        ChangeFocus(nullptr);
    }

}

Direction ButtonNavigator::GetDirectionFromInput() const
{
    for (const auto& [action, key] : keyConfing)
    {
        for (const auto& keycode : key)
        {
            if (input_->IsPadTriggered(static_cast<PadButton>(keycode)) || input_->IsKeyTriggered(keycode))
            {
                switch (action)
                {
                case UIAction::NavigateUp:
                    return Direction::Up;
                case UIAction::NavigateDown:
                    return Direction::Down;
                case UIAction::NavigateLeft:
                    return Direction::Left;
                case UIAction::NavigateRight:
                    return Direction::Right;
                default:
                    break;
                }
            }
        }
    }
    return Direction::None; // デフォルトの方向
}

bool ButtonNavigator::CheckConfirm() const
{
    if (!currentFocusedButton_)
        return false;

    for (const auto& keycode : keyConfing.at(UIAction::Confirm))
    {
        if (input_->IsPadTriggered(static_cast<PadButton>(keycode)) || input_->IsKeyTriggered(keycode))
        {
            currentFocusedButton_->Pressed();
            return true; // 決定が押された
        }
    }

    // マウスクリックチェック
    if (currentFocusedButton_->IsPressed())
        return true;

    return false; // 決定は押されていない
}

void ButtonNavigator::ChangeFocus(UIButton* _button)
{
    if (currentFocusedButton_ != _button)
    {
        if (currentFocusedButton_)
            currentFocusedButton_->SetFocused(false);

        currentFocusedButton_ = _button;
        // nullの可能性があるからnullチェック
        if(currentFocusedButton_)
            currentFocusedButton_->SetFocused(true);
    }
}
