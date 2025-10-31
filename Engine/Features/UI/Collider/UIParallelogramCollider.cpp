#include "UIParallelogramCollider.h"
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Features/UI/UIBase.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/LineDrawer/LineDrawer.h>

bool UIParallelogramCollider::IsPointInside(const Vector2& _point) const
{
    DrawDebug();

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

void UIParallelogramCollider::UpdateCache(const UIBase* _uiBase)
{
    // パラメータを選択（UI依存 or 独立モード）
    Vector2 center, size;
    float rotate;

    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIBaseから情報を取得
        center = _uiBase->GetCenterPos();
        size = _uiBase->GetSize();
        rotate = _uiBase->GetRotate();
    }
    else
    {
        // 独立モード：UIのローカル座標系で計算
        center = _uiBase->GetCenterPos() + localOffset_;
        size = localSize_;
        rotate = _uiBase->GetRotate() + localRotate_;  // UI回転 + ローカル回転
    }

    // 1. スキュー行列（計算前の比率値を使用）
    // スキュー変換行列: [1   sx]   sx = skew_.x (Y座標に対するX方向のずれ)
    //                   [sy  1 ]   sy = skew_.y (X座標に対するY方向のずれ)
    Matrix4x4 skewMatrix = {
        1.0f,    skew_.x, 0.0f, 0.0f,
        skew_.y, 1.0f,    0.0f, 0.0f,
        0.0f,    0.0f,    1.0f, 0.0f,
        0.0f,    0.0f,    0.0f, 1.0f
    };

    // 2. スケール行列（サイズを適用）
    Matrix4x4 scaleMatrix = MakeScaleMatrix({ size.x, size.y, 1.0f });

    // 3. 回転行列（回転を適用）
    Matrix4x4 rotateMatrix = MakeRotateZMatrix(rotate);

    // 4. 平行移動行列（位置を適用）
    Matrix4x4 translateMatrix = MakeTranslateMatrix({ center.x, center.y, 0.0f });

    // 変換行列を合成（重要：右から左へ適用される）
    // 頂点に対して：スキュー → スケール → 回転 → 平行移動 の順で変換
    Matrix4x4 transformMatrix = skewMatrix * scaleMatrix * rotateMatrix * translateMatrix;

    // 正規化された頂点（0.0 ~ 1.0の範囲の単位矩形、中心基準）
    Vector3 normalizedCorners[4] = {
        { -0.5f, -0.5f, 0.0f }, // 左下
        {  0.5f, -0.5f, 0.0f }, // 右下
        {  0.5f,  0.5f, 0.0f }, // 右上
        { -0.5f,  0.5f, 0.0f }  // 左上
    };

    // 変換行列を適用してワールド座標の頂点を計算
    for (int i = 0; i < 4; i++)
    {
        Vector3 worldPos = Transform(normalizedCorners[i], transformMatrix);
        worldCorners_[i] = { worldPos.x, worldPos.y };
    }
}

void UIParallelogramCollider::ImGui()
{
#ifdef _DEBUG
    // トランスフォームモード選択
    const char* modes[] = { "UI Dependent", "Independent" };
    int currentMode = static_cast<int>(transformMode_);
    if (ImGui::Combo("Transform Mode", &currentMode, modes, 2))
    {
        transformMode_ = static_cast<TransformMode>(currentMode);
    }

    // スキュー値（両モード共通）
    ImGui::Separator();
    ImGui::Text("Skew (Common)");
    ImGui::DragFloat2("Skew", &skew_.x, 0.01f, -1.0f, 1.0f);

    // 独立モードの場合のみパラメータ編集可能
    if (transformMode_ == TransformMode::Independent)
    {
        ImGui::Separator();
        ImGui::Text("Independent Parameters (Local)");
        ImGui::DragFloat2("Offset from UI Center", &localOffset_.x, 1.0f);
        ImGui::DragFloat2("Size", &localSize_.x, 1.0f, 0.0f, 1000.0f);
        ImGui::DragFloat("Rotate (Radians)", &localRotate_, 0.01f);
    }
    else
    {
        ImGui::Separator();
        ImGui::TextDisabled("(Using UI parameters)");
    }

    // 計算済みの頂点座標を表示（デバッグ用）
    ImGui::Separator();
    if (ImGui::TreeNode("World Corners"))
    {
        for (int i = 0; i < 4; i++)
        {
            ImGui::Text("Corner[%d]: (%.1f, %.1f)", i, worldCorners_[i].x, worldCorners_[i].y);
        }
        ImGui::TreePop();
    }
#endif
}

void UIParallelogramCollider::DrawDebug() const
{
    for (int i = 0; i < 4; i++)
    {
        Vector2 start = worldCorners_[i];
        Vector2 end = worldCorners_[(i + 1) % 4];
        LineDrawer::GetInstance()->DebugDraw(start, end, Vector4(0, 1, 0, 1));
    }
}
