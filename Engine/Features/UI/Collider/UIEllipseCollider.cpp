#include "UIEllipseCollider.h"

bool UIEllipseCollider::IsPointInside(const Vector2& _point) const
{
    // 中心からの相対座標を計算
    Vector2 diff = _point - center_;

    // 楕円の方程式: (x/a)² + (y/b)² <= 1
    // 各軸の半径で正規化
    float normalizedX = diff.x / radius_.x;
    float normalizedY = diff.y / radius_.y;

    // 方程式の左辺を計算
    float equation = (normalizedX * normalizedX) + (normalizedY * normalizedY);

    // 1以下なら楕円の内側
    return equation <= 1.0f;
}

void UIEllipseCollider::UpdateCache(const UIBase* _uiBase)
{
    // UIの中心位置を取得
    center_ = _uiBase->GetCenterPos();

    // UIのサイズから各軸の半径を計算
    Vector2 size = _uiBase->GetSize();
    radius_ = size * 0.5f;  // X軸とY軸それぞれの半径
}
