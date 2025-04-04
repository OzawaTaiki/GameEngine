#include <Math/Quaternion/Quaternion.h>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

Quaternion Quaternion::Identity()
{
    return Quaternion(0, 0, 0, 1);
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Vector3& _axis, float _angle)
{
    Vector3 nAxis = _axis.Normalize();

    return Quaternion(
        nAxis.x * std::sinf(_angle / 2),
        nAxis.y * std::sinf(_angle / 2),
        nAxis.z * std::sinf(_angle / 2),
        std::cosf(_angle / 2)
    );
}


float Quaternion::Dot(const Quaternion& _q) const
{
    return x * _q.x + y * _q.y + z * _q.z + w * _q.w;
}

Vector3 Quaternion::RotateVector(const Vector3& _v) const
{
    Quaternion vect = Quaternion(_v.x, _v.y, _v.z, 0);
    Quaternion result;
    result = (*this) * vect * Conjugate();
    return Vector3(result.x, result.y, result.z);
}

Matrix4x4 Quaternion::ToMatrix() const
{
    float ww = w * w;
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;

    Matrix4x4 result;

    result.m[0][0] = ww + xx - yy - zz;
    result.m[0][1] = 2 * (x * y + w * z);
    result.m[0][2] = 2 * (x * z - w * y);
    result.m[0][3] = 0;

    result.m[1][0] = 2 * (x * y - w * z);
    result.m[1][1] = ww - xx + yy - zz;
    result.m[1][2] = 2 * (y * z + w * x);
    result.m[1][3] = 0;

    result.m[2][0] = 2 * (x * z + w * y);
    result.m[2][1] = 2 * (y * z - w * x);
    result.m[2][2] = ww - xx - yy + zz;
    result.m[2][3] = 0;

    result.m[3][0] = 0;
    result.m[3][1] = 0;
    result.m[3][2] = 0;
    result.m[3][3] = 1;
    return result;
}

Quaternion Quaternion::Conjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

float Quaternion::Norm() const
{
    return std::sqrtf(x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::Normalize() const
{
    float length = Norm();
    return Quaternion(x / length, y / length, z / length, w / length);
}

Quaternion Quaternion::Inverse() const
{
    return Conjugate() / (x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::FromToRotation(const Vector3& _from, const Vector3& _to)
{
    Vector3 fromNorm = _from.Normalize();
    Vector3 toNorm = _to.Normalize();

    float dot = fromNorm.Dot(toNorm);

    if (dot > 0.99999f)
    {
        // 同じ方向
        return Quaternion(0, 0, 0, 1);
    }
    else if (dot < -0.99999f)
    {
        // 正反対
        Vector3 axis = Vector3(1, 0, 0).Cross(fromNorm);
        if (axis.Dot(axis) < 0.001f)
            axis = Vector3(0, 1, 0).Cross(fromNorm);
        axis = axis.Normalize();
        return Quaternion(axis.x, axis.y, axis.z, 0.0f);
    }

    Vector3 axis = fromNorm.Cross(toNorm).Normalize();

    float angle = std::acos(dot);

    float sinHalf = std::sinf(angle / 2.0f);

    return Quaternion(axis.x * sinHalf, axis.y * sinHalf, axis.z * sinHalf, std::cosf(angle / 2.0f));
}

Quaternion Quaternion::Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
{
    return _q1 * (1.0f - _t) + _q2 * _t;
}

Quaternion Quaternion::Slerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
{
    Quaternion q0 = _q1;
    float dot = _q1.Dot(_q2);
    if (dot < 0.0f)
    {
        q0 = -_q1;
        dot = -dot;
    }
    if (dot > 1.0f - 1e-6f)
    {
        return Lerp(q0, _q2, _t);
    }
    float theta = std::acosf(dot);
    float scale0, scale1;

    scale0 = std::sinf((1.0f - _t) * theta);
    scale1 = std::sinf(_t * theta);

    return (q0 * scale0 + _q2 * scale1) / std::sinf(theta);
}

Quaternion Quaternion::operator-() const
{
    return Quaternion(-x, -y, -z, -w);
}

Quaternion Quaternion::operator*(float _f) const
{
    return Quaternion(x * _f, y * _f, z * _f, w * _f);
}

Quaternion Quaternion::operator/(float _f) const
{
    return Quaternion(x / _f, y / _f, z / _f, w / _f);
}

Quaternion Quaternion::operator+(const Quaternion& _q) const
{
    return Quaternion(x + _q.x, y + _q.y, z + _q.z, w + _q.w);
}

Quaternion Quaternion::operator*(const Quaternion& _q) const
{
    return Quaternion(
        w * _q.x + x * _q.w + y * _q.z - z * _q.y,
        w * _q.y - x * _q.z + y * _q.w + z * _q.x,
        w * _q.z + x * _q.y - y * _q.x + z * _q.w,
        w * _q.w - x * _q.x - y * _q.y - z * _q.z
    );
}
#ifdef _DEBUG
void Quaternion::ShowData(const std::string& _label, bool _newWindow ) const
{
    if(_newWindow)
        ImGui::Begin("Quaternion");

    ImGui::Text("%-12s:%6.2f,%6.2f,%6.2f,%6.2f", _label.c_str(), x, y, z, w);

    if(_newWindow)
        ImGui::End();
}
#endif // _DEBUG
