#pragma once
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
Quaternion Slerp(const Quaternion& _q1, const Quaternion& _q2, float _t);

float Lerp(float _a, float _b, float _t);
Vector2 Lerp(const Vector2& _v1, const Vector2& _v2, float _t);
Vector3 Lerp(const Vector3& _v1, const Vector3& _v2, float _t);
Vector4 Lerp(const Vector4& _v1, const Vector4& _v2, float _t);
Quaternion Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t);

// カラーコードからVector4へ
Vector4 ColorCodeToVector4(uint32_t _colorCode);