#include "UICircleCollider.h"

bool UICircleCollider::IsPointInside(const Vector2& _point) const
{
    // 中心からの距離の2乗を計算（sqrt不要で高速化）
    Vector2 diff = _point - center_;
    float distSquared = diff.x * diff.x + diff.y * diff.y;

    // 半径の2乗と比較
    return distSquared <= (radius_ * radius_);
}

void UICircleCollider::UpdateCache(const UIBase* _uiBase)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIのパラメータから計算
        center_ = _uiBase->GetCenterPos();

        // UIのサイズから半径を計算（短い方の辺の半分）
        Vector2 size = _uiBase->GetSize();
        radius_ = (size.x < size.y) ? (size.x * 0.5f) : (size.y * 0.5f);
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        // UI中心 + ローカルオフセット
        center_ = _uiBase->GetCenterPos() + localOffset_;
        radius_ = independentRadius_;
    }
}

void UICircleCollider::ImGui()
{
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
}
