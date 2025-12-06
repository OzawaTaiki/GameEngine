#include "UIRecntangleCollider.h"
#include <Features/UI/UIElement.h>

#include <Debug/ImGuiDebugManager.h>


bool UIRectangleCollider::IsPointInside(const Vector2& _point) const
{
    return (_point.x >= leftTop_.x && _point.x <= rightBottom_.x &&
            _point.y >= leftTop_.y && _point.y <= rightBottom_.y);
}

void UIRectangleCollider::UpdateCache(const UIElement* _uiElement)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIElementのパラメータから計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();

        leftTop_ = pos;
        rightBottom_ = pos + size;
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();
        Vector2 center = pos + size * 0.5f + localOffset_;
        Vector2 halfSize = localSize_ * 0.5f;

        leftTop_ = center - halfSize;
        rightBottom_ = center + halfSize;
    }
}

void UIRectangleCollider::ImGui()
{
#ifdef _DEBUG
    // トランスフォームモード選択
    const char* modes[] = { "UI Dependent", "Independent" };
    int currentMode = static_cast<int>(transformMode_);
    if (ImGui::Combo("Transform Mode", &currentMode, modes, 2))
    {
        transformMode_ = static_cast<TransformMode>(currentMode);
    }

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

    // デバッグ表示
    ImGui::Separator();
    ImGui::Text("World Position (Debug)");
    ImGui::Text("Left Top: (%.1f, %.1f)", leftTop_.x, leftTop_.y);
    ImGui::Text("Right Bottom: (%.1f, %.1f)", rightBottom_.x, rightBottom_.y);
#endif
}