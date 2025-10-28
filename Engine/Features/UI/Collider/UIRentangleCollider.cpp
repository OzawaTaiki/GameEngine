#include "UIRentangleCollider.h"

bool UIRentangleCollider::IsPointInside(const Vector2& _point) const
{
    // AABB（軸平行境界ボックス）判定
    // 点が左上と右下の範囲内にあるかチェック
    return (_point.x >= leftTop_.x && _point.x <= rightBottom_.x &&
            _point.y >= leftTop_.y && _point.y <= rightBottom_.y);
}

void UIRentangleCollider::UpdateCache(const UIBase* _uiBase)
{
    // UIの左上と右下の座標を取得してキャッシュ
    leftTop_ = _uiBase->GetLeftTopPos();
    rightBottom_ = _uiBase->GetRightBottomPos();
}
