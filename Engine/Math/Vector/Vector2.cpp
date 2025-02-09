#include <Math/Vector/Vector2.h>

Vector2 Vector2::operator+(const Vector2& _v)const
{
    return { x + _v.x,y + _v.y };
}

Vector2 Vector2::operator-(const Vector2& _v)const
{
    return { x - _v.x,y - _v.y };
}

Vector2 Vector2::operator*(float _value)const
{
    return { x * _value,y * _value };
}

Vector2 Vector2::operator*(const Vector2& _v)const
{
    return { x * _v.x,y * _v.y };
}

Vector2 Vector2::operator/(float _value)const
{
    return { x / _value,y / _value };
}

Vector2 Vector2::operator/(const Vector2& _v)const
{
    return { x / _v.x,y / _v.y };
}

Vector2 Vector2::operator-()const
{
    return { -x,-y };
}

float Vector2::Length() const
{
    return std::sqrtf(x * x + y * y);
}

float Vector2::Dot(const Vector2& _v) const
{
    return x * _v.x + y * _v.y;
}

float Vector2::Cross(const Vector2& _v) const
{
    return x * _v.y - y * _v.x;
}

Vector2 Vector2::Normalize() const
{
    float length = Length();
    return { x / length,y / length };
}
