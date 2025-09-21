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

/// <summary>
/// 値のバイアスを計算
/// </summary>
/// <param name="_val">対象の値</param>
/// <param name="_min">最小値</param>
/// <param name="_max">最大値</param>
/// <returns> -ならmin寄り、+ならmax寄り</returns>
float CalculateBias(float _val, float _min, float _max);

float Lerp(float _a, float _b, float _t);
Vector2 Lerp(const Vector2& _v1, const Vector2& _v2, float _t);
Vector3 Lerp(const Vector3& _v1, const Vector3& _v2, float _t);
Vector4 Lerp(const Vector4& _v1, const Vector4& _v2, float _t);
Quaternion Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t);

// カラーコードからVector4へ
Vector4 ColorCodeToVector4(uint32_t _colorCode);