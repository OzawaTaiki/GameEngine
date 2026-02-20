#include <Math/MyLib.h>
#include <algorithm>
#include <numbers>
#include <limits>


namespace Engine {

Quaternion Slerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
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

float CalculateBias(float _val, float _min, float _max)
{
    float center    = (_min + _max) * 0.5f;
    float range     = (_max - _min) * 0.5f;
    return (_val - center) / range;
}

float Lerp(float _a, float _b, float _t)
{
    return _a + (_b - _a) * _t;
}

Vector2 Lerp(const Vector2& _v1, const Vector2& _v2, float _t)
{
    return Vector2(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t));
}

Vector3 Lerp(const Vector3& _v1, const Vector3& _v2, float _t)
{
    return Vector3(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t), Lerp(_v1.z, _v2.z, _t));
}

Vector4 Lerp(const Vector4& _v1, const Vector4& _v2, float _t)
{
    return Vector4(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t), Lerp(_v1.z, _v2.z, _t), Lerp(_v1.w, _v2.w, _t));
}

Quaternion Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
{
	return Quaternion(Lerp(_q1.x, _q2.x, _t), Lerp(_q1.y, _q2.y, _t), Lerp(_q1.z, _q2.z, _t), Lerp(_q1.w, _q2.w, _t));
}
Vector4 ColorCodeToVector4(uint32_t _colorCode)
{
    //0xRRGGBBAA

    float r = static_cast<float>((_colorCode >> 24) & 0xFF) / 255.0f;
    float g = static_cast<float>((_colorCode >> 16) & 0xFF) / 255.0f;
    float b = static_cast<float>((_colorCode >> 8) & 0xFF) / 255.0f;
    float a = static_cast<float>(_colorCode & 0xFF) / 255.0f;

    return Vector4(r, g, b, a);
}

} // namespace Engine
