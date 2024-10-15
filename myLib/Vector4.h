#pragma once
#include <functional>

struct Vector4
{
	float x, y, z, w;

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

