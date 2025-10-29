#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 平行四辺形UIコライダー
/// スキュー変換を用いて平行四辺形の衝突判定を行う
/// UI依存モード：UIのパラメータ + スキュー値から計算
/// 独立モード：独自に設定されたパラメータを使用
/// 外積判定により凸四角形として判定する
/// 行列演算を使用してスキュー・スケール・回転・平行移動を適用
/// </summary>
class UIParallelogramCollider : public IUICollider
{
public:
    UIParallelogramCollider() = default;

    /// <summary>
    /// スキュー値を指定してコライダーを作成
    /// </summary>
    /// <param name="skew">スキュー比率（-1.0 ~ 1.0）X: Y座標に対するX方向のずれ、Y: X座標に対するY方向のずれ</param>
    UIParallelogramCollider(const Vector2& skew) : skew_(skew) {}
    ~UIParallelogramCollider() override = default;

    /// <summary>
    /// 点が平行四辺形の内側にあるかを判定
    /// 外積判定により4つの辺の内側にあるかをチェック
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>平行四辺形の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIBaseの情報から平行四辺形の4頂点をワールド座標で計算してキャッシュする
    /// UI依存モード：UIのパラメータ（位置、サイズ、回転、アンカー）を使用
    /// 独立モード：独自パラメータを使用（UIBaseは無視）
    /// スキュー行列、スケール行列、回転行列、平行移動行列を合成して変換を適用
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    void UpdateCache(const UIBase* _uiBase) override;

    /// <summary>
    /// スキュー値を設定（両モード共通）
    /// </summary>
    /// <param name="_skew">スキュー比率（-1.0 ~ 1.0）</param>
    void SetSkew(const Vector2& _skew) { skew_ = _skew; }

    /// <summary>
    /// 現在のスキュー値を取得
    /// </summary>
    /// <returns>スキュー比率</returns>
    const Vector2& GetSkew() const { return skew_; }

    /// <summary>
    /// ImGuiでスキュー値と頂点座標を表示・編集
    /// </summary>
    void ImGui() override;

    void DrawDebug() const override;

    // 独立モード用パラメータ設定・取得（UIローカル座標系）
    void SetLocalOffset(const Vector2& _offset) { localOffset_ = _offset; }
    void SetLocalSize(const Vector2& _size) { localSize_ = _size; }
    void SetLocalRotate(float _rotate) { localRotate_ = _rotate; }

    Vector2 GetLocalOffset() const { return localOffset_; }
    Vector2 GetLocalSize() const { return localSize_; }
    float GetLocalRotate() const { return localRotate_; }

private:
    // 実際の判定に使用するキャッシュ
    Vector2 worldCorners_[4] = {};  // ワールド座標での4頂点（キャッシュ済み）

    // 共通パラメータ（両モードで使用）
    Vector2 skew_ = { 0.0f, 0.0f }; // スキュー比率（-1.0 ~ 1.0）

    // 独立モード用パラメータ（UIローカル座標系）
    Vector2 localOffset_ = { 0, 0 };      // UI中心からのオフセット
    Vector2 localSize_ = { 100, 100 };    // ローカルサイズ
    float localRotate_ = 0.0f;            // ローカル回転角度（ラジアン）
};
