#include <Math/Vector/Vector2.h>

const Vector2 Vector2::zero = Vector2(0.0f, 0.0f);
const Vector2 Vector2::one = Vector2(1.0f, 1.0f);

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

Vector2& Vector2::operator+=(const Vector2& _v)
{
    x += _v.x;
    y += _v.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& _v)
{
    x -= _v.x;
    y -= _v.y;
    return *this;
}

Vector2& Vector2::operator*=(float _value)
{
    x *= _value;
    y *= _value;
    return *this;
}

Vector2& Vector2::operator/=(float _value)
{
    x /= _value;
    y /= _value;
    return *this;
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
    if (length == 0.0f)return Vector2::zero;
    return { x / length,y / length };
}
