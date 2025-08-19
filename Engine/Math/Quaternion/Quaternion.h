#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>

#include <json.hpp>

#ifdef _DEBUG
#include <string>
#endif // _DEBUG


class Quaternion
{
public:
    float  x, y, z, w;
    Quaternion() : x(0), y(0), z(0), w(1) {}
    Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

    // 単位クォータニオン
    static Quaternion Identity();
    // 軸と角度からクォータニオンを作成
    static Quaternion MakeRotateAxisAngleQuaternion(const Vector3& _axis, float _angle);

    // クォータニオンの内積
    float Dot(const Quaternion& _q) const;
    // ベクトルを回転
    Vector3 RotateVector(const Vector3& _v) const;
    // クォータニオンを行列に変換
    Matrix4x4 ToMatrix() const;
    // 共役
    Quaternion Conjugate() const;
    // ノルム 長さ
    float Norm() const;
    // 正規化
    Quaternion Normalize() const;
    // 逆クォータニオン
    Quaternion Inverse() const;

    static Quaternion FromToRotation(const Vector3& _from, const Vector3& _to);

    // 線形補間
    static Quaternion Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t);

    // 球面線形補間
    static Quaternion Slerp(const Quaternion& _q1, const Quaternion& _q2, float _t);

    // Euler角からクォータニオンを作成
    static Quaternion  EulerToQuaternion(const Vector3& _euler);

    ///****演算子オーバロード****///
    Quaternion operator-() const;
    Quaternion operator* (float _f) const;
    Quaternion operator/ (float _f) const;
    Quaternion operator+ (const Quaternion& _q) const;
    Quaternion operator* (const Quaternion& _q) const;



#ifdef _DEBUG
    void ShowData(const std::string& _label, bool _newWindow = true) const;
#endif // _DEBUG

};
