#pragma once
#include "Vector2.h"
#include <functional>
struct Vector3
{
	float x, y, z;

	inline Vector3() :x(), y(), z() {};
	inline Vector3(float _x) :x(_x), y(0), z(0) {};
	inline Vector3(float _f0, float _f1, float _f2) :x(_f0), y(_f1), z(_f2) {};
	inline Vector3(const Vector2& _v, float _f) :x(_v.x), y(_v.y), z(_f) {};


	inline bool operator==(const Vector3& _v)const {
		return x == _v.x && y == _v.y && z == _v.z;
	}
};

template <>
struct std::hash<Vector3> {
    size_t operator()(const Vector3& v) const {
        size_t h1 = std::hash<float>{}(v.x);
        size_t h2 = std::hash<float>{}(v.y);
        size_t h3 = std::hash<float>{}(v.z);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
};
