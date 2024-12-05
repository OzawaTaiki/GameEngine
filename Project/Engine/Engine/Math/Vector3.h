#pragma once
#include "Vector2.h"
#include <functional>
#ifdef _DEBUG
#include <string>
#endif // _DEBUG

struct Vector3
{
	float x, y, z;

	// コンストラクタ

	inline Vector3()									:x(), y(), z() {};
    // 3つの値が同じ場合
	inline Vector3(float _xyz)							:x(_xyz), y(_xyz), z(_xyz) {};
    // 3つの値が異なる場合
	inline Vector3(float _f0, float _f1, float _f2)		:x(_f0), y(_f1), z(_f2) {};
    // 配列からの初期化
    inline Vector3(const float arr[3])					:x(arr[0]), y(arr[1]), z(arr[2]) {};
    // 2次元ベクトルからの初期化
    inline Vector3(const Vector2& _v)					:x(_v.x), y(_v.y), z(0.0f) {};
    // 2次元ベクトルと1つの値からの初期化
	inline Vector3(const Vector2& _v, float _f)			:x(_v.x), y(_v.y), z(_f) {};
    // コピーコンストラクタ
    inline Vector3(const Vector3& _v)					:x(_v.x), y(_v.y), z(_v.z) {};


    // メンバ関数
    // ベクトルの長さを返す
	float Length()const;
    // ベクトルを正規化する
	Vector3 Normalize() const;
    // ベクトルの内積を返す
	float Dot(const Vector3& _v) const;
    // ベクトルの外積を返す
	Vector3 Cross(const Vector3& _v) const;

   static Vector3 Min(const Vector3& _v1, const Vector3& _v2);
   static Vector3 Max(const Vector3& _v1, const Vector3& _v2);


	inline bool operator==(const Vector3& _v)const {
		return x == _v.x && y == _v.y && z == _v.z;
	}

	Vector3 operator+(const Vector3& _v) const;
	Vector3 operator-(const Vector3& _v) const;
	Vector3 operator*(const Vector3& _v) const;
	Vector3 operator/(const Vector3& _v) const;
	Vector3 operator*(float _s) const;
	Vector3 operator/(float _s) const;
	Vector3 operator-() const;
	Vector3& operator+= (const Vector3& _v);
	Vector3& operator-= (const Vector3& _v);
    Vector3& operator*= (const Vector3& _v);
    Vector3& operator/= (const Vector3& _v);


    inline operator float* () { return &x; }
	inline operator const float* () const { return &x; }
    //Vector3& operator= (const float arr[3]);
	

	friend Vector3 operator*(float _s, const Vector3& vec);
	friend Vector3 operator/(float _s, const Vector3& vec);

    void Vec3ToFloat3(float _f[3]) const;

#ifdef _DEBUG
	void ShowData(const std::string& _label, bool _newWindow = true) const;
#endif // _DEBUG
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
