#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 円形UIコライダー
/// 中心からの距離で衝突判定を行う
/// UI依存モード：UIのサイズから自動的に半径を計算する（短い方の辺の半分）
/// 独立モード：独自に設定された中心と半径を使用
/// </summary>
class UICircleCollider : public IUICollider
{
public:
    UICircleCollider() = default;
    ~UICircleCollider() override = default;

    /// <summary>
    /// 点が円の内側にあるかを判定
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>円の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIElementの情報から円の中心と半径をキャッシュする
    /// UI依存モード：UIのサイズの短い方の辺の半分として計算される
    /// 独立モード：独自パラメータを使用（UIElementは無視）
    /// </summary>
    /// <param name="_element">親となるUIElement</param>
    void UpdateCache(const UIElement *_element) override;

    /// <summary>
    /// ImGuiでパラメータを表示・編集する
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// 独立モード用：円の中心オフセットを設定（UIローカル座標系）
    /// </summary>
    /// <param name="_offset">UI中心からのオフセット</param>
    void SetLocalOffset(const Vector2& _offset) { localOffset_ = _offset; }

    /// <summary>
    /// 独立モード用：円の半径を設定
    /// </summary>
    /// <param name="_radius">円の半径</param>
    void SetRadius(float _radius) { independentRadius_ = _radius; }

    /// <summary>
    /// 独立モード用：円の中心オフセットを取得
    /// </summary>
    /// <returns>UI中心からのオフセット</returns>
    Vector2 GetLocalOffset() const { return localOffset_; }

    /// <summary>
    /// 独立モード用：円の半径を取得
    /// </summary>
    /// <returns>円の半径</returns>
    float GetIndependentRadius() const { return independentRadius_; }


    void DrawDebug() const override;
private:
    // 実際の判定に使用するキャッシュ
    Vector2 center_ = { 0, 0 };  // 円の中心（ワールド座標）
    float radius_ = 0.0f;        // 円の半径

    // 独立モード用パラメータ（UIローカル座標系）
    Vector2 localOffset_ = { 0, 0 };        // UI中心からのオフセット
    float independentRadius_ = 100.0f;      // 独立モードでの半径
};