#pragma once

struct Vector2
{
	float x, y;

	inline bool operator==(const Vector2& _v) const{
		return x == _v.x && y == _v.y;
	}
};