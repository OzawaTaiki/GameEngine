#include "UIButtonComponent.h"
#include <Features/UI/Component/UIColliderComponent.h>
#include <Features/UI/Collider/UICollisionManager.h>
#include <Features/UI/Element/UIElement.h>
#include <System/Input/Input.h>

#ifdef _DEBUG
#include <Debug/ImGuiDebugManager.h>
#endif


namespace Engine {

void UIButtonComponent::Initialize()
{
    // ColliderComponentを取得、なければ自動追加
    collider_ = owner_->GetComponent<UIColliderComponent>();
    if (!collider_)
    {
        collider_ = owner_->AddComponent<UIColliderComponent>(ColliderType::Rectangle);
    }
    collider_->GetCollider()->UpdateCache(owner_);
}

void UIButtonComponent::Update()
{
    if (!isEnabled_ || !owner_->IsEnabled())
    {
        state_ = ButtonState::Disabled;
        return;
    }

    UpdateState();
}

void UIButtonComponent::DrawImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    static std::string stateNames[] = { "Normal", "Hovered", "Pressed", "Disabled" };
    if (ImGui::TreeNode("UIButtonComponent"))
    {
        ImGui::Checkbox("Enabled", &isEnabled_);
        ImGui::Text("State: %s", stateNames[static_cast<int>(state_)].c_str());
        ImGui::BeginDisabled(1);
        bool callback = (onClick_ != nullptr);
        ImGui::Checkbox("has Callback", &callback);
        ImGui::EndDisabled();
        ImGui::TreePop();
    }
    ImGui::PopID();
#endif
}

void UIButtonComponent::SetEnabled(bool enabled)
{
    isEnabled_ = enabled;
    if (!enabled)
    {
        state_ = ButtonState::Disabled;
    }
}

void UIButtonComponent::UpdateState()
{
    if (!collider_ || !collider_->GetCollider())
        return;

    auto* input = Input::GetInstance();
    bool isHit = collider_->GetCollider()->GetIsHit();
    ButtonState prevState = state_;

    if (isHit)
    {
        // Hover状態に入った瞬間
        if (prevState == ButtonState::Normal)
        {
            if (onHoverEnter_) onHoverEnter_();
            if (onHover_) onHover_();  // 既存の互換性維持
        }

        // Hover中（毎フレーム）
        if (onHovering_) onHovering_();

        // マウスボタンが押された瞬間
        if (input->IsMouseTriggered(static_cast<uint8_t>(MouseButton::Left)))
        {
            state_ = ButtonState::Pressed;
            if (onClickDown_) onClickDown_();
        }
        // マウスボタンが押されている間
        else if (input->IsMousePressed(static_cast<uint8_t>(MouseButton::Left)))
        {
            state_ = ButtonState::Pressed;
        }
        // マウスボタンが離された
        else
        {
            state_ = ButtonState::Hovered;
        }

        // クリックが完了した瞬間（ボタン内でリリース）
        if (prevState == ButtonState::Pressed && state_ == ButtonState::Hovered)
        {
            if (onClickUp_) onClickUp_();
            else if (onClick_) onClick_();  // 既存の互換性維持
        }
    }
    else
    {
        // Hover状態から出た瞬間
        if (prevState == ButtonState::Hovered || prevState == ButtonState::Pressed)
        {
            if (onHoverExit_) onHoverExit_();
        }

        state_ = ButtonState::Normal;
    }
}

void UIButtonComponent::Execute()
{
    if (!isEnabled_ || !owner_->IsEnabled())
        return;

    if (onClick_)
    {
        onClick_();
    }
}

} // namespace Engine
