#include "UIEllipseCollider.h"
#include <Features/UI/UIElement.h>

#include <Debug/ImGuiDebugManager.h>



namespace Engine {

bool UIEllipseCollider::IsPointInside(const Vector2& _point) const
{
    // 中心からの相対座標を計算
    Vector2 diff = _point - center_;

    // 楕円の方程式: (x/a)² + (y/b)² <= 1
    // 各軸の半径で正規化
    float normalizedX = diff.x / radius_.x;
    float normalizedY = diff.y / radius_.y;

    // 方程式の左辺を計算
    float equation = (normalizedX * normalizedX) + (normalizedY * normalizedY);

    // 1以下なら楕円の内側
    return equation <= 1.0f;
}

void UIEllipseCollider::UpdateCache(const UIElement* _uiElement)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIのパラメータから計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();
        center_ = pos + size * 0.5f;  // 中心座標を計算

        // UIのサイズから各軸の半径を計算
        radius_ = size * 0.5f;  // X軸とY軸それぞれの半径
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

void UIEllipseCollider::ImGui()
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
        ImGui::DragFloat2("Radius (X, Y)", &independentRadius_.x, 1.0f, 0.0f, 1000.0f);
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
    ImGui::Text("Radius: (%.1f, %.1f)", radius_.x, radius_.y);
#endif // _DEBUG
}

} // namespace Engine
