#include "Ray.h"
#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>


Ray Ray::CreateFromStartAndEnd(const Vector3& _start, const Vector3& _end)
{
    Ray ray;

    ray.origin_ = _start;
    ray.direction_ = (_end - _start).Normalize();
    ray.length_ = (_end - _start).Length();

    return ray;

}

Ray Ray::CreateFromPointAndDirection(const Vector3& _point, const Vector3& _direction)
{
    Ray ray;

    ray.origin_ = _point;
    ray.direction_ = _direction.Normalize();

    return ray;
}

Ray Ray::CreateFromPointAndTarget(const Vector3& _point, const Vector3& _target)
{
    Ray ray;

    ray.origin_ = _point;
    ray.direction_ = (_target - _point).Normalize();
    ray.length_ = (_target - _point).Length();

    return ray;
}

Ray Ray::CreateFromMouseCursor(const Camera& _camera, const Vector2& _mousePos, uint32_t _screenWidth, uint32_t _screenHeight)
{
    // 0~1の範囲に正規化
    float normalizedX = _mousePos.x / _screenWidth;
    float normalizedY = _mousePos.y / _screenHeight;

    // -1~1の範囲に変換
    float ndcX = normalizedX * 2.0f - 1.0f;
    float ndcY = 1.0f - normalizedY * 2.0f;

    float tanFovY = std::tanf(_camera.fovY_ * 0.5f) * 3.14159265f / 180.0f;

    Vector3 rayDir;
    rayDir.x = ndcX * tanFovY * _camera.aspectRatio_;
    rayDir.y = ndcY * tanFovY;
    rayDir.z = 1.0f;

    Matrix4x4 rotate = MakeRotateMatrix(_camera.rotate_);
    rayDir = Transform(rayDir, rotate).Normalize();

    return CreateFromPointAndDirection(_camera.translate_, rayDir);

}

