#pragma once

#include <Features/UI/Collider/Interface/IUICollider.h>

#include <vector>

/// <summary>
/// 凸多角形コライダークラス
/// </summary>
/// <remarks>
/// 凸多角形のUIコライダーを表すクラス
/// 頂点の順序は時計回りまたは反時計回りで指定する
/// ※凸形状でない場合，判定が正しく動作しない可能性あり
/// </remarks>
 class UIConvexPolygonCollider : public IUICollider
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    UIConvexPolygonCollider() = default;
    /// <summary>
    /// デストラクタ
    /// </summary>
    ~UIConvexPolygonCollider() override = default;

    /// <summary>
    /// 点が多角形の内側にあるかを判定する
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>多角形の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIElementの情報から多角形の頂点をキャッシュする
    /// </summary>
    /// <param name="_uiElement">親となるUIElement</param>
    void UpdateCache(const UIElement* _uiElement) override;

    /// <summary>
    /// ImGuiでパラメータを表示・編集する
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// ローカル座標系での頂点リストを設定する
    /// </summary>
    /// <param name="_index">頂点インデックス</param>
    /// <param name="_vertex">頂点座標</param>
    void SetLocalVertex(size_t _index, const Vector2& _vertex);

    /// <summary>
    /// ローカル座標系での頂点リストを設定する
    /// </summary>
    /// <param name="_vertices">頂点リスト</param>
    void SetLocalVertices(const std::vector<Vector2>& _vertices) { localVertices_ = _vertices; }

    /// <summary>
    /// ローカル座標系での頂点を取得する
    /// </summary>
    /// <param name="_index"> 頂点インデックス</param>
    /// <returns> 頂点座標 範囲外の場合は(0,0)を返す</returns>
    const std::vector<Vector2>& GetLocalVertices() const { return localVertices_; }

    void DrawDebug() const override;

private:
    std::vector<Vector2> localVertices_;   // ローカル座標系での頂点リスト
    std::vector<Vector2> worldVertices_;   // ワールド座標系での頂点リスト
 };
