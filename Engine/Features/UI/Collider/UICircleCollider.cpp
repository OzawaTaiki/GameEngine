#include "UICircleCollider.h"
#include <Features/UI/Element/UIElement.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/LineDrawer/LineDrawer.h>

bool UICircleCollider::IsPointInside(const Vector2& _point) const
{
    Vector2 diff = _point - center_;
    float distSquared = diff.x * diff.x + diff.y * diff.y;
    return distSquared <= (radius_ * radius_);
}

void UICircleCollider::UpdateCache(const UIElement* _uiElement)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIのパラメータから計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();
        center_ = pos + size * 0.5f;  // 中心座標

        // UIのサイズから半径を計算（短い方の辺の半分）
        radius_ = (size.x < size.y) ? (size.x * 0.5f) : (size.y * 0.5f);
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();
        center_ = pos + size * 0.5f + localOffset_;
        radius_ = independentRadius_;
    }
}
void UICircleCollider::ImGui()
{
#ifdef _DEBUG
    // トランスフォームモード選択
    const char* modes[] = { "UI Dependent", "Independent" };
    int currentMode = static_cast<int>(transformMode_);
    if (ImGui::Combo("Transform Mode", &currentMode, modes, 2))
    {
        transformMode_ = static_cast<TransformMode>(currentMode);
    }

    // 独立モードの場合のみパラメータ編集可能
    if (transformMode_ == TransformMode::Independent)
    {
        ImGui::Separator();
        ImGui::Text("Independent Parameters (Local)");
        ImGui::DragFloat2("Offset from UI Center", &localOffset_.x, 1.0f);
        ImGui::DragFloat("Radius", &independentRadius_, 1.0f, 0.0f, 1000.0f);
    }
    else
    {
        ImGui::Separator();
        ImGui::TextDisabled("(Using UI parameters)");
    }

    // 実際のワールド座標を表示（デバッグ用）
    ImGui::Separator();
    ImGui::Text("World Position (Debug)");
    ImGui::Text("Center: (%.1f, %.1f)", center_.x, center_.y);
    ImGui::Text("Radius: %.1f", radius_);
#endif // _DEBUG
}

void UICircleCollider::DrawDebug() const
{
    LineDrawer::GetInstance()->DebugDrawCircle(center_, radius_);
}
