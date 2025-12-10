#include "UINavigationComponent.h"
#include <Features/UI/UIElement.h>

#ifdef _DEBUG
#include <Debug/ImGuiDebugManager.h>
#endif

void UINavigationComponent::Initialize()
{
    // 初期化処理（現状特になし）
}

void UINavigationComponent::Update()
{
    // 更新処理（現状特になし）
}

void UINavigationComponent::DrawImGui()
{
#ifdef _DEBUG
    if (ImGui::TreeNode("UINavigationComponent"))
    {
        ImGui::Checkbox("Focusable", &focusable_);
        ImGui::Checkbox("Is Focused", &isFocused_);

        ImGui::Separator();
        ImGui::Text("Navigation:");

        // Navigation先の表示
        ImGui::Text("Up:    %s", up_ ? up_->GetName().c_str() : "None");
        ImGui::Text("Down:  %s", down_ ? down_->GetName().c_str() : "None");
        ImGui::Text("Left:  %s", left_ ? left_->GetName().c_str() : "None");
        ImGui::Text("Right: %s", right_ ? right_->GetName().c_str() : "None");

        ImGui::TreePop();
    }
#endif
}

void UINavigationComponent::SetNavigation(NavigationDirection dir, UIElement* target)
{
    switch (dir)
    {
    case NavigationDirection::Up:
        up_ = target;
        break;
    case NavigationDirection::Down:
        down_ = target;
        break;
    case NavigationDirection::Left:
        left_ = target;
        break;
    case NavigationDirection::Right:
        right_ = target;
        break;
    }
}

UIElement* UINavigationComponent::GetNext(NavigationDirection dir) const
{
    switch (dir)
    {
    case NavigationDirection::Up:
        return up_;
    case NavigationDirection::Down:
        return down_;
    case NavigationDirection::Left:
        return left_;
    case NavigationDirection::Right:
        return right_;
    default:
        return nullptr;
    }
}
