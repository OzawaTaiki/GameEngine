#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 楕円形UIコライダー
/// 楕円の方程式を用いて衝突判定を行う
/// UIのサイズから自動的にX軸とY軸の半径を計算する
/// </summary>
class UIEllipseCollider : public IUICollider
{
public:
    UIEllipseCollider() = default;
    ~UIEllipseCollider() override = default;

    /// <summary>
    /// 点が楕円の内側にあるかを判定
    /// 楕円の方程式 (x/a)² + (y/b)² <= 1 を使用
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>楕円の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIBaseの情報から楕円の中心と半径をキャッシュする
    /// X軸の半径 = width / 2、Y軸の半径 = height / 2
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    void UpdateCache(const UIBase* _uiBase) override;

private:
    Vector2 center_ = { 0, 0 };  // 楕円の中心（ワールド座標）
    Vector2 radius_ = { 0.0f, 0.0f };  // X軸とY軸の半径
};