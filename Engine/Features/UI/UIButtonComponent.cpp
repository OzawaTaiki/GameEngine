#include "UIButtonComponent.h"
#include <Features/UI/Collider/UIColliderComponent.h>
#include <Features/UI/Collider/UICollisionManager.h>
#include <Features/UI/UIElement.h>
#include <System/Input/Input.h>

#ifdef _DEBUG
#include <Debug/ImGuiDebugManager.h>
#endif

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

    // UICollisionManagerにコライダーを登録
    if (collider_ && collider_->GetCollider())
    {
        auto collider = collider_->GetCollider();
        collider->UpdateCache(owner_);
        UICollisionManager::GetInstance()->RegisterElement(
            collider,
            owner_->GetOrder()
        );
#ifdef _DEBUG
        collider->DrawDebug();
#endif

    }

    UpdateState();
}

void UIButtonComponent::DrawImGui()
{
#ifdef _DEBUG
    if (ImGui::TreeNode("UIButtonComponent"))
    {
        // 状態表示
        const char* stateNames[] = { "Normal", "Hovered", "Pressed", "Disabled" };
        ImGui::Text("State: %s", stateNames[static_cast<int>(state_)]);

        // 有効/無効トグル
        ImGui::Checkbox("Enabled", &isEnabled_);

        // コールバック設定状態
        ImGui::Text("OnClick: %s", onClick_ ? "Set" : "Not Set");
        ImGui::Text("OnHover: %s", onHover_ ? "Set" : "Not Set");

        ImGui::TreePop();
    }
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

    // UICollisionManagerの結果を取得
    bool isHit = collider_->GetCollider()->GetIsHit();

    ButtonState prevState = state_;

    if (isHit)
    {
        // マウスオーバー中
        if (input->IsMousePressed(static_cast<uint8_t>(MouseButton::Left)))
        {
            // クリック中
            state_ = ButtonState::Pressed;
        }
        else
        {
            // ホバー中
            state_ = ButtonState::Hovered;

            // Hovered状態に入った瞬間
            if (prevState == ButtonState::Normal && onHover_)
            {
                onHover_();
            }
        }

        // クリックが離された瞬間（ボタン内で）
        if (prevState == ButtonState::Pressed &&
            state_ == ButtonState::Hovered &&
            onClick_)
        {
            onClick_();
        }
    }
    else
    {
        // マウスオーバーしていない
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
