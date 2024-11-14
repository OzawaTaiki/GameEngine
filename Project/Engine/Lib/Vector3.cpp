#include "Vector3.h"


float Vector3::Length() const
{
    return std::sqrtf(x * x + y * y + z * z);

}

Vector3 Vector3::Normalize() const
{
    float length = Length();
    if (length == 0.0f)
        return *this;
    return *this / length;
}

float Vector3::Dot(const Vector3& _v) const
{
    float result = x * _v.x + y * _v.y + z * _v.z;

    return result;
}

Vector3 Vector3::Cross(const Vector3& _v) const
{
    Vector3 result;

    if (*this == Vector3{0, 0, 0})
        return _v;
    else if (_v == Vector3{ 0, 0, 0 })
        return *this;

    result.x = y * _v.z - z * _v.y;
    result.y = z * _v.x - x * _v.z;
    result.z = x * _v.y - y * _v.x;

    return result;
}

Vector3 Vector3::operator+(const Vector3& _v) const
{
    return Vector3(x + _v.x, y + _v.y, z + _v.z);
}

Vector3 Vector3::operator-(const Vector3& _v) const
{
    return Vector3(x - _v.x, y - _v.y, z - _v.z);
}

Vector3 Vector3::operator*(const Vector3& _v) const
{
    return Vector3(x * _v.x, y * _v.y, z * _v.z);
}

Vector3 Vector3::operator/(const Vector3& _v) const
{
    return Vector3(x / _v.x, y / _v.y, z / _v.z);
}

Vector3 Vector3::operator*(float _s) const
{
    return Vector3(x * _s, y * _s, z * _s);
}

Vector3 Vector3::operator/(float _s) const
{
    return Vector3(x / _s, y / _s, z / _s);
}

Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}

Vector3& Vector3::operator+=(const Vector3& _v)
{
    *this = *this + _v;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& _v)
{
    *this = *this - _v;
    return *this;
}

Vector3 operator*(float _s, const Vector3& vec)
{
    return { vec * _s };
}

Vector3 operator/(float _s, const Vector3& vec)
{
    return { vec / _s };
}

Vector3& Vector3::operator*=(float _s)
{
    *this = *this * _s;
    return *this;
}


#ifdef _DEBUG
#include <imgui.h>
void Vector3::ShowData(const std::string& _label, bool _newWindow) const
{
    if (_newWindow)
        ImGui::Begin((_label + " Vector3").c_str());

    ImGui::Text("%-12s:%6.3f,%6.3f,%6.3f", _label.c_str(), x, y, z);

    if (_newWindow)
        ImGui::End();
}
#endif // _DEBUG
