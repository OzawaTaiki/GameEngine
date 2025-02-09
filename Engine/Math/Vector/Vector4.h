#pragma once
#include <Math/Vector/Vector3.h>
#include <functional>

#include <json.hpp>

struct Vector4
{
	float x, y, z, w;

    inline Vector4() :x(), y(), z(),w() {};
    inline Vector4(float _x) :x(_x), y(0), z(0),w(0) {};
    inline Vector4(float _f0, float _f1, float _f2, float _f3) :x(_f0), y(_f1), z(_f2), w(_f3) {};
    inline Vector4(const Vector3& _v, float _f) :x(_v.x), y(_v.y), z(_v.z), w(_f) {};

    Vector4 operator-()const { return Vector4(-x, -y, -z, -w); }
    Vector4 operator+(const Vector4& _v)const { return Vector4(x + _v.x, y + _v.y, z + _v.z, w + _v.w); }
    Vector4 operator-(const Vector4& _v)const { return Vector4(x - _v.x, y - _v.y, z - _v.z, w - _v.w); }
    Vector4 operator*(const Vector4& _v)const { return Vector4(x * _v.x, y * _v.y, z * _v.z, w * _v.w); }
    Vector4 operator/(const Vector4& _v)const { return Vector4(x / _v.x, y / _v.y, z / _v.z, w / _v.w); }
    Vector4 operator*(float _f)const { return Vector4(x * _f, y * _f, z * _f, w * _f); }
    Vector4 operator/(float _f)const { return Vector4(x / _f, y / _f, z / _f, w / _f); }
    Vector4& operator*=(float _f) { x *= _f; y *= _f; z *= _f; w *= _f; return *this; }
    Vector4& operator/=(float _f) { x /= _f; y /= _f; z /= _f; w /= _f; return *this; }

    Vector2 xy()const { return Vector2(x, y); }
    Vector3 xyz()const { return Vector3(x, y, z); }


    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Vector4, x, y, z, w)


    inline void operator =(const Vector3& _v) {
        x = _v.x;
        y = _v.y;
        z = _v.z;
    }

	inline bool operator==(const Vector4& _v)const{
		return x == _v.x && y == _v.y && z == _v.z && w == _v.w;
	}
};


template <>
struct std::hash<Vector4> {
    size_t operator()(const Vector4& v) const {
        size_t h1 = std::hash<float>{}(v.x);
        size_t h2 = std::hash<float>{}(v.y);
        size_t h3 = std::hash<float>{}(v.z);
        size_t h4 = std::hash<float>{}(v.w);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1) ^ (h4 << 2);
    }
};

