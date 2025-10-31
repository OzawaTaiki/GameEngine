#include "UIQuadCollider.h"
#include <Features/LineDrawer/LineDrawer.h>
#include <Features/UI/UIBase.h>

bool UIQuadCollider::IsPointInside(const Vector2& _point) const
{
    // 外積判定（点が4つの辺の内側にあるかチェック）
    // 凸四角形の各辺に対して、点が内側にあるかを判定
    for (int i = 0; i < 4; i++)
    {
        // 辺のベクトルを計算（頂点i -> 頂点i+1）
        Vector2 edge = worldCorners_[(i + 1) % 4] - worldCorners_[i];

        // 頂点iから判定点へのベクトルを計算
        Vector2 toPoint = _point - worldCorners_[i];

        // 外積の計算（2D外積のZ成分）
        // 正の値 = 点が辺の左側（内側）
        // 負の値 = 点が辺の右側（外側）
        float cross = edge.x * toPoint.y - edge.y * toPoint.x;

        // 外側にある場合はfalse
        if (cross < 0.0f)
            return false;
    }

    // すべての辺の内側にあればtrue
    return true;
}

void UIQuadCollider::UpdateCache(const UIBase* _uiBase)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIの左上・右下座標から矩形として計算
        Vector2 leftTop = _uiBase->GetLeftTopPos();
        Vector2 rightBottom = _uiBase->GetRightBottomPos();

        worldCorners_[0] = { leftTop.x, rightBottom.y };      // 左下
        worldCorners_[1] = { rightBottom.x, rightBottom.y };  // 右下
        worldCorners_[2] = { rightBottom.x, leftTop.y };      // 右上
        worldCorners_[3] = { leftTop.x, leftTop.y };          // 左上
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        // UI中心 + 各頂点のローカルオフセット
        Vector2 uiCenter = _uiBase->GetCenterPos();
        for (int i = 0; i < 4; i++)
        {
            worldCorners_[i] = uiCenter + localCorners_[i];
        }
    }
}

void UIQuadCollider::ImGui()
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
        ImGui::Text("Corner Positions from UI Center:");

        const char* cornerNames[] = { "Left-Bottom", "Right-Bottom", "Right-Top", "Left-Top" };
        for (int i = 0; i < 4; i++)
        {
            ImGui::PushID(i);
            ImGui::DragFloat2(cornerNames[i], &localCorners_[i].x, 1.0f);
            ImGui::PopID();
        }
    }
    else
    {
        ImGui::Separator();
        ImGui::TextDisabled("(Using UI parameters)");
    }

    // 実際のワールド座標を表示（デバッグ用）
    ImGui::Separator();
    if (ImGui::TreeNode("World Corners (Debug)"))
    {
        for (int i = 0; i < 4; i++)
        {
            ImGui::Text("Corner[%d]: (%.1f, %.1f)", i, worldCorners_[i].x, worldCorners_[i].y);
        }
        ImGui::TreePop();
    }
#endif
}

void UIQuadCollider::DrawDebug() const
{
    // 4つの辺を線描画
    for (int i = 0; i < 4; i++)
    {
        Vector2 start = worldCorners_[i];
        Vector2 end = worldCorners_[(i + 1) % 4];
        LineDrawer::GetInstance()->DebugDraw(start, end, Vector4(0, 1, 0, 1));  // 黄色
    }
}

void UIQuadCollider::SetLocalCorner(int _index, const Vector2& _corner)
{
    if (_index >= 0 && _index < 4)
    {
        localCorners_[_index] = _corner;
    }
}

Vector2 UIQuadCollider::GetLocalCorner(int _index) const
{
    if (_index >= 0 && _index < 4)
    {
        return localCorners_[_index];
    }
    return Vector2();
}
