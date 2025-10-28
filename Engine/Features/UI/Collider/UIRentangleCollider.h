#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 矩形（長方形）UIコライダー
/// 回転なしの軸平行境界ボックス（AABB）として衝突判定を行う
/// 最も基本的で高速なコライダー
/// </summary>
class UIRentangleCollider : public IUICollider
{
public:
    UIRentangleCollider() = default;
    ~UIRentangleCollider() override = default;

    /// <summary>
    /// 点が矩形の内側にあるかを判定
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>矩形の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIBaseの情報から矩形の左上と右下の座標をキャッシュする
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    void UpdateCache(const UIBase* _uiBase) override;

private:
    Vector2 leftTop_ = { 0, 0 };      // 矩形の左上座標（ワールド座標）
    Vector2 rightBottom_ = { 0, 0 };  // 矩形の右下座標（ワールド座標）
};
