#pragma once
#include "Vector3.h"
#include <functional>

struct Vector4
{
	float x, y, z, w;

    inline Vector4() :x(), y(), z(),w() {};
    inline Vector4(float _x) :x(_x), y(0), z(0),w(0) {};
    inline Vector4(float _f0, float _f1, float _f2, float _f3) :x(_f0), y(_f1), z(_f2), w(_f3) {};
    inline Vector4(const Vector3& _v, float _f) :x(_v.x), y(_v.y), z(_v.z), w(_f) {};

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

