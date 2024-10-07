#pragma once

struct Vector4
{
	float x, y, z, w;

	inline bool operator==(const Vector4& _v)const{
		return x == _v.x && y == _v.y && z == _v.z && w == _v.w;
	}
};