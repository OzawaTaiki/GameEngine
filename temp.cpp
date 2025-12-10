#include "UINavigationManager.h"
#include <Features/UI/UIElement.h>
#include <Features/UI/UINavigationComponent.h>
#include <Features/UI/UIButtonComponent.h>
#include <System/Input/Input.h>
#include <algorithm>

UINavigationManager* UINavigationManager::GetInstance()
{
    static UINavigationManager instance;
    return &instance;
}

void UINavigationManager::Register(UIElement* element)
{
    if (!element)
        return;

    // 既に登録されているかチェック
    auto it = std::find(focusableElements_.begin(), focusableElements_.end(), element);
    if (it == focusableElements_.end())
    {
        focusableElements_.push_back(element);
    }
}

void UINavigationManager::Unregister(UIElement* element)
{
    if (!element)
        return;

    auto it = std::find(focusableElements_.begin(), focusableElements_.end(), element);
    if (it != focusableElements_.end())
    {
        focusableElements_.erase(it);
    }

    // 現在のフォーカスが削除される要素だった場合
    if (currentFocus_ == element)
    {
        currentFocus_ = nullptr;
    }
}

void UINavigationManager::HandleInput()
{
    auto* input = Input::GetInstance();

    // フォーカスがない場合は何もしない
    if (!currentFocus_)
        return;

    auto* navComp = GetNavigationComponent(currentFocus_);
    if (!navComp || !navComp->IsFocusable())
        return;

    // 方向キー入力でフォーカス移動
    UIElement* nextElement = nullptr;

    if (input->IsKeyTriggered(DIK_UP) || input->IsPadTriggered(PadButton::iPad_Up))
    {
        nextElement = navComp->GetNext(NavigationDirection::Up);
    }
    else if (input->IsKeyTriggered(DIK_DOWN) || input->IsPadTriggered(PadButton::iPad_Down))
    {
        nextElement = navComp->GetNext(NavigationDirection::Down);
    }
    else if (input->IsKeyTriggered(DIK_LEFT) || input->IsPadTriggered(PadButton::iPad_Left))
    {
        nextElement = navComp->GetNext(NavigationDirection::Left);
    }
    else if (input->IsKeyTriggered(DIK_RIGHT) || input->IsPadTriggered(PadButton::iPad_Right))
    {
        nextElement = navComp->GetNext(NavigationDirection::Right);
    }

    // 次の要素にフォーカス移動
    if (nextElement)
    {
        SetFocus(nextElement);
    }

    // 決定キー（Enter / ゲームパッドA）で実行
    if (input->IsKeyTriggered(DIK_RETURN) || input->IsPadTriggered(PadButton::iPad_A))
    {
        ExecuteCurrentElement();
    }
}

void UINavigationManager::SetFocus(UIElement* element)
{
    if (!element)
        return;

    auto* navComp = GetNavigationComponent(element);
    if (!navComp || !navComp->IsFocusable())
        return;

    // 以前のフォーカスを解除
    if (currentFocus_)
    {
        auto* prevNavComp = GetNavigationComponent(currentFocus_);
        if (prevNavComp)
        {
            prevNavComp->SetFocused(false);
        }
    }

    // 新しいフォーカスを設定
    currentFocus_ = element;
    navComp->SetFocused(true);
}

void UINavigationManager::ClearFocus()
{
    if (currentFocus_)
    {
        auto* navComp = GetNavigationComponent(currentFocus_);
        if (navComp)
        {
            navComp->SetFocused(false);
        }
        currentFocus_ = nullptr;
    }
}


void UINavigationManager::ExecuteCurrentElement()
{
    if (!currentFocus_)
        return;

    // ボタンコンポーネントを持っている場合、クリック処理を実行
    auto* buttonComp = currentFocus_->GetComponent<UIButtonComponent>();
    if (buttonComp)
    {
        buttonComp->Execute();
    }
}

UINavigationComponent* UINavigationManager::GetNavigationComponent(UIElement* element) const
{
    if (!element)
        return nullptr;

    return element->GetComponent<UINavigationComponent>();
}
