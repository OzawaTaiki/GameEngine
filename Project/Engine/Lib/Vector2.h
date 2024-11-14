#pragma once
#include <functional>

struct Vector2
{
	float x, y;

    Vector2 operator-(const Vector2& _v);

    inline Vector2 operator/(float _value) {
        return { x / _value,y / _value };
   }

	inline bool operator==(const Vector2& _v) const{
		return x == _v.x && y == _v.y;
	}

    float Length() const;
    Vector2 Normalize() const;
};

template <>
struct std::hash<Vector2> {
    size_t operator()(const Vector2& v) const {
        size_t h1 = std::hash<float>{}(v.x);
        size_t h2 = std::hash<float>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};
