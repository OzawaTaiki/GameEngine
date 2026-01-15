#include "UIConvexPolygonCollider.h"
#include <Features/UI/UIElement.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiDebugManager.h>


namespace Engine {

bool UIConvexPolygonCollider::IsPointInside(const Vector2& _point) const
{

    // 外積判定（点がすべての辺の内側にあるかチェック）
    size_t vertexCount = worldVertices_.size();

    if (vertexCount < 3)
    {
        // 頂点が3つ未満の場合、多角形として成立しないためfalseを返す
        return false;
    }

    for (size_t i = 0; i < vertexCount; i++)
    {
        // 辺のベクトルを計算（頂点i -> 頂点i+1）
        Vector2 edge = worldVertices_[(i + 1) % vertexCount] - worldVertices_[i];
        if (edge == Vector2(0.0f, 0.0f))
            return false;

        // 頂点iから判定点へのベクトルを計算
        Vector2 toPoint = _point - worldVertices_[i];
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

void UIConvexPolygonCollider::UpdateCache(const UIElement* _uiElement)
{
    if (transformMode_ == TransformMode::UIDependent)
    {
        // UI依存モード：UIの左上・右下座標から矩形として計算
        Vector2 pos = _uiElement->GetWorldPosition();
        Vector2 size = _uiElement->GetSize();
        Vector2 leftTop = pos;
        Vector2 rightBottom = pos + size;

        worldVertices_.clear();
        const size_t defaultVertexCount = 4;
        worldVertices_.resize(defaultVertexCount);

        worldVertices_[0] = { leftTop.x, rightBottom.y };      // 左下
        worldVertices_[1] = { rightBottom.x, rightBottom.y };  // 右下
        worldVertices_[2] = { rightBottom.x, leftTop.y };      // 右上
        worldVertices_[3] = { leftTop.x, leftTop.y };          // 左上
    }
    else if (transformMode_ == TransformMode::Independent)
    {
        // 独立モード：UIのローカル座標系で計算
        Vector2 pos = _uiElement->GetWorldPosition();
        worldVertices_.resize(localVertices_.size());
        for (size_t i = 0; i < localVertices_.size(); i++)
        {
            worldVertices_[i] = pos + localVertices_[i];
        }
    }
}

void UIConvexPolygonCollider::ImGui()
{
#ifdef _DEBUG

    std::vector<bool> removeFlags(localVertices_.size(), false);

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
        ImGui::Text("Independent Parameters (Local Vertices):");
        for (size_t i = 0; i < localVertices_.size(); i++)
        {
            ImGui::PushID(static_cast<int>(i));
            ImGui::DragFloat2(("Vertex " + std::to_string(i)).c_str(), &localVertices_[i].x, 1.0f);
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                removeFlags[i] = true;
            }
            ImGui::PopID();
        }
        if (ImGui::Button("Add Vertex"))
        {
            localVertices_.emplace_back(0.0f, 0.0f);
        }
    }
    else
    {
        ImGui::Separator();
        ImGui::TextDisabled("(Using UI parameters)");
    }
    // 実際のワールド座標を表示（デバッグ用）
    ImGui::Separator();
    if (ImGui::TreeNode("World Vertices (Debug)"))
    {
        for (size_t i = 0; i < worldVertices_.size(); i++)
        {
            ImGui::Text("Vertex[%d]: (%.1f, %.1f)", static_cast<int>(i), worldVertices_[i].x, worldVertices_[i].y);
        }
        ImGui::TreePop();
    }

    for (int i = static_cast<int>(removeFlags.size()) - 1; i >= 0; i--)
    {
        if (removeFlags[i])
        {
            localVertices_.erase(localVertices_.begin() + i);
        }
    }

#endif
}

void UIConvexPolygonCollider::SetLocalVertex(size_t _index, const Vector2& _vertex)
{
    if (_index >= localVertices_.size())
    {// インデックスが範囲外の場合は末尾に追加
        localVertices_.push_back(_vertex);
    }
    else
    {
        localVertices_[_index] = _vertex;
    }
}

void UIConvexPolygonCollider::DrawDebug() const
{
    auto lineDrawer = LineDrawer::GetInstance();

    // 各辺を線描画
    size_t vertexCount = worldVertices_.size();
    for (size_t i = 0; i < vertexCount; i++)
    {
        Vector2 start = worldVertices_[i];
        Vector2 end = worldVertices_[(i + 1) % vertexCount];
        lineDrawer->DebugDraw(start, end, Vector4(0, 1, 0, 1)); // 緑色
    }
}

} // namespace Engine
