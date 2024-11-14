#include "Vector2.h"

Vector2 Vector2::operator-(const Vector2& _v)
{
    return { x - _v.x,y - _v.y };
}

float Vector2::Length() const
{
    return std::sqrtf(x * x + y * y);
}

Vector2 Vector2::Normalize() const
{
    float length = Length();
    return { x / length,y / length };
}
