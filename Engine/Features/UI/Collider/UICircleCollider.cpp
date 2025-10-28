#include "UICircleCollider.h"

bool UICircleCollider::IsPointInside(const Vector2& _point) const
{
    // 中心からの距離の2乗を計算（sqrt不要で高速化）
    Vector2 diff = _point - center_;
    float distSquared = diff.x * diff.x + diff.y * diff.y;

    // 半径の2乗と比較
    return distSquared <= (radius_ * radius_);
}

void UICircleCollider::UpdateCache(const UIBase* _uiBase)
{
    // UIの中心位置を取得
    center_ = _uiBase->GetCenterPos();

    // UIのサイズから半径を計算（短い方の辺の半分）
    Vector2 size = _uiBase->GetSize();
    radius_ = (size.x < size.y) ? (size.x * 0.5f) : (size.y * 0.5f);
}
