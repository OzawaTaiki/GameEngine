#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 円形UIコライダー
/// 中心からの距離で衝突判定を行う
/// UIのサイズから自動的に半径を計算する（短い方の辺の半分）
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
    /// UIBaseの情報から円の中心と半径をキャッシュする
    /// 半径はUIのサイズの短い方の辺の半分として計算される
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    void UpdateCache(const UIBase* _uiBase) override;

private:
    Vector2 center_ = { 0, 0 };  // 円の中心（ワールド座標）
    float radius_ = 0.0f;        // 円の半径
};