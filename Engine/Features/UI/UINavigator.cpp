#include "UINavigator.h"
#include <System/Input/Input.h>
#include <algorithm>

UINavigator::KeyConfig UINavigator::keyConfig_ =
{
    {UIAction::NavigateUp,      {DIK_UP, DIK_W, static_cast<int32_t>(PadButton::iPad_Up)}},
    {UIAction::NavigateDown,    {DIK_DOWN, DIK_S, static_cast<int32_t>(PadButton::iPad_Down)}},
    {UIAction::NavigateLeft,    {DIK_LEFT, DIK_A, static_cast<int32_t>(PadButton::iPad_Left)}},
    {UIAction::NavigateRight,   {DIK_RIGHT, DIK_D, static_cast<int32_t>(PadButton::iPad_Right)}},
    {UIAction::Confirm,         {DIK_RETURN, DIK_SPACE, static_cast<int32_t>(PadButton::iPad_A),static_cast<int32_t>(MouseButton::Left)}}
};

void UINavigator::Initialize(UISelectable* _focusedElement)
{
    input_ = Input::GetInstance();
    currentFocused_ = _focusedElement;

    if (currentFocused_)
    {
        currentFocused_->SetFocused(true);
    }
}

void UINavigator::Update()
{
    if (!input_)
        return;

    // マウスの移動検出閾値
    const float mouseMoveThreshold = 1.0f;
    // マウスの位置を取得
    Vector2 mousePosition = input_->GetMousePosition();
    if ((mousePosition - previousMousePosition_).Length() > mouseMoveThreshold)
    {
        // マウスが動いた場合、マウスによるフォーカスをチェック
        CheckElementFocus();
    }

    // 方向キー入力処理
    if (Direction dir = GetDirectionFromInput(); dir != Direction::None)
    {
        if (currentFocused_)
        {
            if (UISelectable* nextFocus = currentFocused_->GetNavigationTarget(dir); nextFocus && nextFocus->IsFocusable())
            {
                ChangeFocus(nextFocus);
            }
        }
        else
        {
            // フォーカスがない場合、最初の要素にフォーカス
            if (!registeredElements_.empty())
            {
                for (auto* element : registeredElements_)
                {
                    if (element && element->IsFocusable())
                    {
                        ChangeFocus(element);
                        break;
                    }
                }
            }
        }
    }

    CheckConfirm();

    previousMousePosition_ = mousePosition;
}

void UINavigator::RegisterSelectable(UISelectable* _element)
{
    if (_element)
    {
        registeredElements_.push_back(_element);
        _element->SetConfirmKeys(keyConfig_[UIAction::Confirm]);
    }
}

void UINavigator::UnregisterSelectable(UISelectable* _element)
{
    if (!_element)
        return;

    auto it = std::find(registeredElements_.begin(), registeredElements_.end(), _element);
    if (it != registeredElements_.end())
    {
        registeredElements_.erase(it);

        // フォーカスしていた要素が削除された場合
        if (currentFocused_ == _element)
        {
            currentFocused_ = nullptr;
        }
    }
}

void UINavigator::ClearSelectables()
{
    registeredElements_.clear();
    currentFocused_ = nullptr;
}

void UINavigator::CheckElementFocus()
{
    // 判定があったか
    bool isFocused = false;
    for (auto* element : registeredElements_)
    {
        if (element && element->IsFocusable() && element->IsMousePointerInside())
        {
            ChangeFocus(element);
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

Direction UINavigator::GetDirectionFromInput() const
{
    for (const auto& [action, keys] : keyConfig_)
    {
        for (const auto& keycode : keys)
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
    return Direction::None;
}

bool UINavigator::CheckConfirm() const
{
    if (!currentFocused_)
        return false;

    // フォーカスされた要素の入力処理を呼び出し
    return currentFocused_->HandleInput();
}

void UINavigator::ChangeFocus(UISelectable* _element)
{
    if (currentFocused_ != _element)
    {
        if (currentFocused_)
            currentFocused_->SetFocused(false);

        currentFocused_ = _element;

        // nullの可能性があるからnullチェック
        if (currentFocused_ && currentFocused_->IsFocusable())
            currentFocused_->SetFocused(true);
    }
}