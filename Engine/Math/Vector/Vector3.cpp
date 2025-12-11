#include <Math/Vector/Vector3.h>

#include <Math/Quaternion/Quaternion.h>
#include <numbers>

const Vector3 Vector3::zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::one = Vector3(1.0f, 1.0f, 1.0f);

float Vector3::Length() const
{
    return std::sqrtf(LengthSquared());

}

float Vector3::LengthSquared() const
{
    return x * x + y * y + z * z;
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

Vector3 Vector3::Min(const Vector3& _v1, const Vector3& _v2)
{
    Vector3 result;

    result.x = std::min(_v1.x, _v2.x);
    result.y = std::min(_v1.y, _v2.y);
    result.z = std::min(_v1.z, _v2.z);

    return result;
}

Vector3 Vector3::Max(const Vector3& _v1, const Vector3& _v2)
{
    Vector3 result;

    result.x = std::max(_v1.x, _v2.x);
    result.y = std::max(_v1.y, _v2.y);
    result.z = std::max(_v1.z, _v2.z);

    return result;
}

Vector3 Vector3::Lerp(const Vector3& _v1, const Vector3& _v2, float _t)
{
    Vector3 result;
    result = _v1 + (_v2 - _v1) * _t;
    return result;
}

Vector3 Vector3::QuaternionToEuler(const Quaternion& _q)
{
    Vector3 euler;

    // ジンバルロックチェック
    float sinY = 2.0f * (_q.w * _q.y - _q.z * _q.x);

    if (abs(sinY) >= 0.99999f) {
        // ジンバルロック状態
        euler.x = atan2(2.0f * (_q.x * _q.y + _q.w * _q.z),
            1.0f - 2.0f * (_q.y * _q.y + _q.z * _q.z));
        euler.y = copysign(std::numbers::pi_v<float> / 2.0f, sinY);
        euler.z = 0.0f;
    }
    else {
        euler.x = atan2(2.0f * (_q.w * _q.x + _q.y * _q.z),
            1.0f - 2.0f * (_q.x * _q.x + _q.y * _q.y));
        euler.y = asin(sinY);
        euler.z = atan2(2.0f * (_q.w * _q.z + _q.x * _q.y),
            1.0f - 2.0f * (_q.y * _q.y + _q.z * _q.z));
    }

    return euler;
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

Vector3& Vector3::operator*=(const Vector3& _v)
{
    *this = *this * _v;
    return *this;
}

Vector3& Vector3::operator/=(const Vector3& _v)
{
    *this = *this / _v;
    return *this;

}

//Vector3& Vector3::operator=(const float arr[3])
//{
//    x = arr[0];
//    y = arr[1];
//    z = arr[2];
//    return *this;
//}

Vector3 operator*(float _s, const Vector3& vec)
{
    return { vec * _s };
}

Vector3 operator/(float _s, const Vector3& vec)
{
    return { vec / _s };
}

void Vector3::Vec3ToFloat3(float _f[3]) const
{
    _f[0] = x;
    _f[1] = y;
    _f[2] = z;

    return;
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
