#include "UIParallelogramCollider.h"
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

#ifdef _DEBUG
#include <imgui.h>
#endif
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
    // UIBaseから必要な情報を取得
    Vector2 center = _uiBase->GetCenterPos();
    Vector2 size = _uiBase->GetSize();
    float rotate = _uiBase->GetRotate();
    Vector2 anchor = _uiBase->GetAnchor();

    // 1. スキュー行列（計算前の比率値を使用）
    // スキュー変換行列: [1   sx]   sx = skew_.x (Y座標に対するX方向のずれ)
    //                   [sy  1 ]   sy = skew_.y (X座標に対するY方向のずれ)
    Matrix4x4 skewMatrix = {
        1.0f,    skew_.x, 0.0f, 0.0f,
        skew_.y, 1.0f,    0.0f, 0.0f,
        0.0f,    0.0f,    1.0f, 0.0f,
        0.0f,    0.0f,    0.0f, 1.0f
    };

    // 2. スケール行列（UIのサイズを適用）
    Matrix4x4 scaleMatrix = MakeScaleMatrix({ size.x,size.y, 1.0f });

    // 3. 回転行列（UIの回転を適用）
    Matrix4x4 rotateMatrix = MakeRotateZMatrix(rotate);

    // 4. 平行移動行列（UIの位置を適用）
    Matrix4x4 translateMatrix = MakeTranslateMatrix({ center.x, center.y, 0.0f });

    // 変換行列を合成（重要：右から左へ適用される）
    // 頂点に対して：スキュー → スケール → 回転 → 平行移動 の順で変換
    Matrix4x4 transformMatrix = skewMatrix * scaleMatrix * rotateMatrix * translateMatrix;

    // 正規化された頂点（-0.5 ~ 0.5の範囲の単位矩形)
    Vector3 normalizedCorners[4] = {
        { 0.0f, 0.0f, 0.0f }, // 左下
        { 1.0f, 0.0f, 0.0f }, // 右下
        { 1.0f, 1.0f, 0.0f }, // 右上
        { 0.0f, 1.0f, 0.0f }  // 左上
    };

    // アンカーオフセットを適用（アンカーポイントの調整）
    for (int i = 0; i < 4; i++)
    {
        normalizedCorners[i].x -= anchor.x;
        normalizedCorners[i].y -= anchor.y;
    }

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
    // 計算済みの頂点座標を表示（デバッグ用）
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
        LineDrawer::GetInstance()->RegisterPoint(start, end, Vector4(0, 1, 0, 1));
    }
}
