#include "Rect.h"

Rect Rect::FromPoints(const Vector2& leftTop, const Vector2& rightBottom)
{
    return Rect(leftTop, rightBottom - leftTop);
}

Vector2 Rect::GetLeftTop() const
{
    return leftTop;
}

Vector2 Rect::GetRightBottom() const
{
    return leftTop + size;
}

Vector2 Rect::GetCenter() const
{
    return leftTop + (size * 0.5f);
}

Vector2 Rect::GetPointAt(const Vector2& normalizedPos) const
{
    return GetPointAt(normalizedPos.x, normalizedPos.y);
}

Vector2 Rect::GetPointAt(float normalizedX, float normalizedY) const
{
    return leftTop + Vector2(size.x * normalizedX, size.y * normalizedY);
}

bool Rect::Contains(const Vector2& point) const
{
    return (point.x >= leftTop.x) && (point.x <= leftTop.x + size.x) &&
            (point.y >= leftTop.y) && (point.y <= leftTop.y + size.y);
}
