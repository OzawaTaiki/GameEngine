#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 四角形UIコライダー（4頂点自由設定）
/// 4つの頂点を自由に設定できる汎用的なコライダー
/// UI依存モード：UIのサイズから矩形として計算
/// 独立モード：4頂点を個別に設定可能（UIローカル座標系）
/// 外積判定により凸四角形として判定する
/// </summary>
class UIQuadCollider : public IUICollider
{
public:
    UIQuadCollider() = default;
    ~UIQuadCollider() override = default;

    /// <summary>
    /// 点が四角形の内側にあるかを判定
    /// 外積判定により4つの辺の内側にあるかをチェック
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>四角形の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIElementの情報から四角形の4頂点をワールド座標で計算してキャッシュする
    /// UI依存モード：UIのサイズから矩形として計算
    /// 独立モード：ローカル座標系で設定された4頂点を使用
    /// </summary>
    /// <param name="_uiElement">親となるUIElement</param>
    void UpdateCache(const UIElement* _uiElement) override;

    /// <summary>
    /// ImGuiでパラメータを表示・編集する
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// コライダーの形状をデバッグ描画する
    /// </summary>
    void DrawDebug() const override;

    /// <summary>
    /// 独立モード用：特定の頂点を設定（UIローカル座標系）
    /// </summary>
    /// <param name="_index">頂点インデックス（0-3: 左下、右下、右上、左上）</param>
    /// <param name="_corner">頂点座標</param>
    void SetLocalCorner(int _index, const Vector2& _corner);

    /// <summary>
    /// 独立モード用：特定の頂点を取得
    /// </summary>
    /// <param name="_index">頂点インデックス（0-3）</param>
    /// <returns>頂点座標</returns>
    Vector2 GetLocalCorner(int _index) const;

private:
    // 実際の判定に使用するキャッシュ
    Vector2 worldCorners_[4] = {};  // ワールド座標での4頂点（キャッシュ済み）

    // 独立モード用パラメータ（UIローカル座標系）
    // デフォルトは100x100の矩形（中心基準）
    Vector2 localCorners_[4] = {
        { -50, -50 },  // 左下
        {  50, -50 },  // 右下
        {  50,  50 },  // 右上
        { -50,  50 }   // 左上
    };
};
