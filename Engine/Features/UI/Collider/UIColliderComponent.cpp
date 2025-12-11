#include "UIColliderComponent.h"
#include <Features/UI/UIElement.h>
#include <Features/UI/Collider/UIRecntangleCollider.h>
#include <Features/UI/Collider/UICircleCollider.h>
#include <Features/UI/Collider/UIEllipseCollider.h>
#include <Features/UI/Collider/UIQuadCollider.h>
#include <Features/UI/Collider/UIConvexPolygonCollider.h>
#include <cassert>
#include <iostream>

#include <Debug/ImGuiDebugManager.h>
#include <Debug/Debug.h>
#include "UICollisionManager.h"

UIColliderComponent::UIColliderComponent(ColliderType type)
    : colliderType_(type)
{
    collider_ = UIColliderFactory::Create(type);
}

void UIColliderComponent::Initialize()
{
    assert(owner_ != nullptr);
    assert(collider_ != nullptr);

    owner_->RegisterVariable("colliderType", reinterpret_cast<uint32_t*>(&colliderType_));

    // サイズ未設定なら自動設定
    InitializeColliderSize();

#ifdef _DEBUG

    Debug::Log(std::format("[UIColliderComponent] Initialized on '{}', Type: {}",
                           owner_->GetName(),
                           UIColliderFactory::GetTypeName(colliderType_)));
#endif
}

void UIColliderComponent::InitializeColliderSize()
{
    Vector2 ownerSize = owner_->GetSize();

    switch (colliderType_)
    {
    case ColliderType::Rectangle:
    {
        auto* rect = static_cast<UIRectangleCollider*>(collider_.get());
        // サイズが{0,0}なら自動設定
        if (rect->GetLocalSize() == Vector2(0, 0))
        {
            rect->SetLocalSize(ownerSize);
        }
        break;
    }

    case ColliderType::Circle:
    {
        auto* circle = static_cast<UICircleCollider*>(collider_.get());
        // 半径が0なら自動設定
        if (circle->GetIndependentRadius() == 0.0f)
        {
            float radius = (ownerSize.x < ownerSize.y) ? (ownerSize.x * 0.5f) : (ownerSize.y * 0.5f);
            circle->SetRadius(radius);
        }
        break;
    }

    case ColliderType::Ellipse:
    {
        auto* ellipse = static_cast<UIEllipseCollider*>(collider_.get());
        // 半径が{0,0}なら自動設定
        if (ellipse->GetIndependentRadius() == Vector2(0, 0))
        {
            ellipse->SetRadius(ownerSize * 0.5f);
        }
        break;
    }

    // Quad, ConvexPolygon は手動設定が前提なので何もしない
    default:
        break;
    }

}

void UIColliderComponent::Update()
{
    assert(collider_ != nullptr);

    // UIElementを直接渡してキャッシュ更新
    collider_->UpdateCache(owner_);
    UICollisionManager::GetInstance()->RegisterElement(
        collider_.get(),
        owner_->GetOrder()
    );

    // デバッグ描画
    if (debugDraw_)
    {
        collider_->DrawDebug();
    }
}

bool UIColliderComponent::IsHit(const Vector2& worldPos) const
{
    assert(collider_ != nullptr);
    return collider_->IsPointInside(worldPos);
}

void UIColliderComponent::SetColliderType(ColliderType type)
{
    if (colliderType_ == type) return;

    colliderType_ = type;
    collider_ = UIColliderFactory::Create(type);

    if (owner_)
    {
        InitializeColliderSize();
    }
}

void UIColliderComponent::SetSize(const Vector2& size)
{
    if (colliderType_ == ColliderType::Rectangle)
    {
        auto* rect = static_cast<UIRectangleCollider*>(collider_.get());
        rect->SetLocalSize(size);
    }
}

Vector2 UIColliderComponent::GetSize() const
{
    if (colliderType_ == ColliderType::Rectangle)
    {
        auto* rect = static_cast<const UIRectangleCollider*>(collider_.get());
        return rect->GetLocalSize();
    }
    return Vector2(0, 0);
}

void UIColliderComponent::SetOffset(const Vector2& offset)
{
    switch (colliderType_)
    {
    case ColliderType::Rectangle:
        static_cast<UIRectangleCollider*>(collider_.get())->SetLocalOffset(offset);
        break;
    case ColliderType::Circle:
        static_cast<UICircleCollider*>(collider_.get())->SetLocalOffset(offset);
        break;
    case ColliderType::Ellipse:
        static_cast<UIEllipseCollider*>(collider_.get())->SetLocalOffset(offset);
        break;
    default:
        break;
    }
}

Vector2 UIColliderComponent::GetOffset() const
{
    switch (colliderType_)
    {
    case ColliderType::Rectangle:
        return static_cast<const UIRectangleCollider*>(collider_.get())->GetLocalOffset();
    case ColliderType::Circle:
        return static_cast<const UICircleCollider*>(collider_.get())->GetLocalOffset();
    case ColliderType::Ellipse:
        return static_cast<const UIEllipseCollider*>(collider_.get())->GetLocalOffset();
    default:
        return Vector2(0, 0);
    }
}

void UIColliderComponent::DrawImGui()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    if (ImGui::TreeNode("UIColliderComponent"))
    {
        static std::string typeNames[] = {
            "Rectangle", "Circle", "Ellipse", "Quad", "ConvexPolygon"
        };
        ImGui::Text("Collider Type: %s", typeNames[static_cast<int>(colliderType_)].c_str());
        ImGui::Checkbox("Debug Draw", &debugDraw_);

        ImGui::TreeNode("Collider Details");
        collider_->ImGui();

        ImGui::TreePop();
    }
    ImGui::PopID();

#endif
}
