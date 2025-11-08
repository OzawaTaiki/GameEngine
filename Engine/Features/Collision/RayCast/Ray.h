#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>

#include <limits>
class Camera;
class Ray
{
public:
    static Ray CreateFromStartAndEnd(const Vector3& _start, const Vector3& _end);

    static Ray CreateFromPointAndDirection(const Vector3& _point, const Vector3& _direction);

    static Ray CreateFromPointAndTarget(const Vector3& _point, const Vector3& _target);

    static Ray CreateFromMouseCursor(const Camera& _camera, const Vector2& _mousePos, uint32_t _screenWidth = 1280, uint32_t _screenHeight = 720);

    // コンストラクタ
    Ray() : origin_({ 0,0,0 }), direction_({ 0,0,1 }), length_((std::numeric_limits<float>::max)()) {}
    // コンストラクタ
    //Ray(const Vector3& _origin, const Vector3& _direction, float _length = (std::numeric_limits<float>::max)()) : origin_(_origin), direction_(_direction) {}

    // デストラクタ
    ~Ray() = default;


    // レイの原点を設定する
    void SetOrigin(const Vector3& _origin) { origin_ = _origin; }
    // レイの原点を取得する
    Vector3 GetOrigin() const { return origin_; }

    // レイの方向を設定する
    void SetDirection(const Vector3& _direction) { direction_ = _direction; }
    // レイの方向を取得する
    Vector3 GetDirection() const { return direction_; }

    // レイの長さを設定する
    void SetLength(float _length) { length_ = _length; }
    // レイの長さを取得する
    float GetLength() const { return length_; }


    // レイの指定距離の点を取得する
    Vector3 GetPoint(float _distance) const { return origin_ + direction_ * _distance; }

private:
    Vector3 origin_; // レイの原点
    Vector3 direction_; // レイの方向
    float length_;
};