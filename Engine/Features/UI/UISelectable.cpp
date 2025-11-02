#include "UISelectable.h"
#include <Debug/Debug.h>
#include <System/Input/Input.h>

UISelectable::UISelectable()
{
}

void UISelectable::Initialize(const std::string& _label, bool _regsterDebugWindow)
{
    UIInteractive::Initialize(_label, _regsterDebugWindow);

    jsonBinder_->RegisterVariable("defaultColor", &defaultColor_);
    jsonBinder_->RegisterVariable("hoverColor", &hoverColor_);
    jsonBinder_->RegisterVariable("pressedColor", &pressedColor_);
    jsonBinder_->RegisterVariable("selectedColor", &selectedColor_);
    jsonBinder_->RegisterVariable("focusedColor", &focusedColor_);
}

void UISelectable::UpdateSelf()
{
    UIInteractive::UpdateSelf();

    Input* input = Input::GetInstance();
    Vector2 mousePos = input->GetMousePosition();

    // ドラッグ処理
    if (isHovered_ && input->IsMouseTriggered(0))
    {
        isDragging_ = true;
        dragStartPos_ = mousePos;
        dragOffset_ = mousePos - GetWorldPos();
        OnDragStart();
    }

    if (isDragging_)
    {
        if (input->IsMousePressed(0) || input->IsMouseTriggered(0))
        {
            // ドラッグ中
            OnDragging();
        }
        else
        {
            // ドラッグ終了
            isDragging_ = false;
            OnDragEnd();
        }
    }

    if (isFocused_)
    {
        // フォーカスがある場合、ナビゲーション処理
        ProcessNavigation();
    }

    color_ = GetCurrentColor();
}

void UISelectable::SetFocused(bool _focused)
{
    if (isFocused_ == _focused)
        return;

    isFocused_ = _focused;

    if (isFocused_)
    {
        OnFocusGained();
    }
    else
    {
        OnFocusLost();
    }
}

void UISelectable::SetNavigationTarget(UISelectable* _target, Direction _dir)
{
    switch (_dir)
    {
        case Direction::Up:
            upTarget_ = _target;
            _target->downTarget_ = this; // 双方向に設定
            break;
        case Direction::Down:
            downTarget_ = _target;
            _target->upTarget_ = this; // 双方向に設定
            break;
        case Direction::Left:
            leftTarget_ = _target;
            _target->rightTarget_ = this; // 双方向に設定
            break;
        case Direction::Right:
            rightTarget_ = _target;
            _target->leftTarget_ = this; // 双方向に設定
            break;
        default:
            break;
    }
}

UISelectable* UISelectable::GetNavigationTarget(Direction _dir) const
{
    switch (_dir)
    {
        case Direction::Up:
            return upTarget_;
        case Direction::Down:
            return downTarget_;
        case Direction::Left:
            return leftTarget_;
        case Direction::Right:
            return rightTarget_;
        default:
            return nullptr;
    }
}


void UISelectable::ImGuiContent()
{
#ifdef _DEBUG

    UIInteractive::ImGuiContent();

    if (ImGui::TreeNode("UISelectable"))
    {
        ImGui::ColorEdit4("Default Color", &defaultColor_.x);
        ImGui::ColorEdit4("Hover Color", &hoverColor_.x);
        ImGui::ColorEdit4("Pressed Color", &pressedColor_.x);
        ImGui::ColorEdit4("Selected Color", &selectedColor_.x);
        ImGui::ColorEdit4("Focused Color", &focusedColor_.x);

        ImGui::Text("Functions:");
        std::string str = onDragStartCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" onDragStart" + str).c_str());
        str = onDraggingCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" onDragging" + str).c_str());
        str = onDragEndCallback_ ? " [Set]" : " [Not Set]";
        ImGui::Text((" onDragEnd" + str).c_str());

        ImGui::Text("Navigation Targets:");
        ImGui::Text((" Up: " + (upTarget_ ? upTarget_->label_ : "None")).c_str());
        ImGui::Text((" Down: " + (downTarget_ ? downTarget_->label_ : "None")).c_str());
        ImGui::Text((" Left: " + (leftTarget_ ? leftTarget_->label_ : "None")).c_str());
        ImGui::Text((" Right: " + (rightTarget_ ? rightTarget_->label_ : "None")).c_str());


        ImGui::TreePop();
    }

#endif // _DEBUG
}

void UISelectable::OnDragStart()
{
    if (onDragStartCallback_)
    {
        onDragStartCallback_();
    }
}

void UISelectable::OnDragging()
{
    if (onDraggingCallback_)
    {
        onDraggingCallback_();
    }
}

void UISelectable::OnDragEnd()
{
    if (onDragEndCallback_)
    {
        onDragEndCallback_();
    }
}

void UISelectable::OnFocusGained()
{
    // フォーカス時の処理
    // 派生クラスでオーバーライド可能
}

void UISelectable::OnFocusLost()
{
    // フォーカス喪失時の処理
    // 派生クラスでオーバーライド可能
}

Vector4 UISelectable::GetCurrentColor() const
{
    // 優先順位: Pressed/Dragging > Focused > Selected > Hovered > Default
    if (isPressed_ || isDragging_)
        return pressedColor_;

    if (isFocused_)
        return focusedColor_;

    if (isSelected_)
        return selectedColor_;

    if (isHovered_)
        return hoverColor_;

    return defaultColor_;
}

void UISelectable::ProcessNavigation()
{
    Input* input = Input::GetInstance();
    if (input->IsKeyTriggered(DIK_UP))
    {
        if (upTarget_)
        {
            SetFocused(false);
            upTarget_->SetFocused(true);
        }
    }
    if (input->IsKeyTriggered(DIK_DOWN))
    {
        if (downTarget_)
        {
            SetFocused(false);
            downTarget_->SetFocused(true);
        }
    }
    if (input->IsKeyTriggered(DIK_LEFT))
    {
        if (leftTarget_)
        {
            SetFocused(false);
            leftTarget_->SetFocused(true);
        }
    }
    if (input->IsKeyTriggered(DIK_RIGHT))
    {
        if (rightTarget_)
        {
            SetFocused(false);
            rightTarget_->SetFocused(true);
        }
    }
}
