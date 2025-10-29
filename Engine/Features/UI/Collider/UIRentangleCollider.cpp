#include "UIRentangleCollider.h"

bool UIRentangleCollider::IsPointInside(const Vector2& _point) const
{
    // AABB（軸平行境界ボックス）判定
    // 点が左上と右下の範囲内にあるかチェック
    return (_point.x >= leftTop_.x && _point.x <= rightBottom_.x &&
            _point.y >= leftTop_.y && _point.y <= rightBottom_.y);
}

void UIRentangleCollider::UpdateCache(const UIBase* _uiBase)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIのパラメータから計算
        leftTop_ = _uiBase->GetLeftTopPos();
        rightBottom_ = _uiBase->GetRightBottomPos();
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        // UI中心 + オフセット ± サイズの半分
        Vector2 center = _uiBase->GetCenterPos() + localOffset_;
        Vector2 halfSize = localSize_ * 0.5f;
        leftTop_ = center - halfSize;
        rightBottom_ = center + halfSize;
    }
}

void UIRentangleCollider::ImGui()
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
        ImGui::DragFloat2("Size", &localSize_.x, 1.0f, 0.0f, 1000.0f);
    }
    else
    {
        ImGui::Separator();
        ImGui::TextDisabled("(Using UI parameters)");
    }

    // 実際のワールド座標を表示（デバッグ用）
    ImGui::Separator();
    ImGui::Text("World Position (Debug)");
    ImGui::Text("Left Top: (%.1f, %.1f)", leftTop_.x, leftTop_.y);
    ImGui::Text("Right Bottom: (%.1f, %.1f)", rightBottom_.x, rightBottom_.y);
}
