#include "Collider.h"
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>


bool SphereCollider::Contains(const Vector3& _point) const
{
    return _point.Length() <= radius_;
}

Vector3 SphereCollider::GetClosestPoint(const Vector3& _point) const
{
    return _point.Normalize() * radius_;
}

bool AABBCollider::Contains(const Vector3& _point) const
{
    return min_.x <= _point.x && _point.x <= max_.x &&
        min_.y <= _point.y && _point.y <= max_.y &&
        min_.z <= _point.z && _point.z <= max_.z;
}

Vector3 AABBCollider::GetClosestPoint(const Vector3& _point) const
{
    return Vector3(
        std::clamp(_point.x, min_.x, max_.x),
        std::clamp(_point.y, min_.y, max_.y),
        std::clamp(_point.z, min_.z, max_.z)
    );
}

bool OBBCollider::Contains(const Vector3& _point) const
{
    Vector3 center = GetCenter();
    WorldTransform transform = *GetWorldTransform();
    Matrix4x4 invRotMat = Inverse(transform.quaternion_.ToMatrix());

    Vector3 localPoint = Transform(_point - center, invRotMat);

    return std::abs(localPoint.x) <= halfExtents_.x &&
        std::abs(localPoint.y) <= halfExtents_.y &&
        std::abs(localPoint.z) <= halfExtents_.z;
}

Vector3 OBBCollider::GetClosestPoint(const Vector3& _point) const
{
    Vector3 center = GetCenter();
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 rotMat = transform.quaternion_.ToMatrix();

    Matrix4x4 invRotMat = Inverse(rotMat);
    Vector3 localPoint = Transform(_point - center, invRotMat);

    // ローカル空間でクランプ
    Vector3 localClosest(
        std::clamp(localPoint.x, -halfExtents_.x, halfExtents_.x),
        std::clamp(localPoint.y, -halfExtents_.y, halfExtents_.y),
        std::clamp(localPoint.z, -halfExtents_.z, halfExtents_.z)
    );

    // ワールド空間に戻す
    return center + Transform(localClosest, rotMat);
}

std::vector<Vector3> OBBCollider::GetVertices() const
{
    std::vector<Vector3> corners(8);
    Vector3 center = GetCenter(); // ピボットではなく実際の中心を使用
    WorldTransform transform = *GetWorldTransform();
    Matrix4x4 rotMat = transform.quaternion_.ToMatrix();

    // ローカル空間での頂点（中心を原点とする）
    Vector3 localCorners[8] = {
        Vector3(-halfExtents_.x, -halfExtents_.y, -halfExtents_.z), // 0: 左下後
        Vector3(halfExtents_.x, -halfExtents_.y, -halfExtents_.z), // 1: 右下後
        Vector3(halfExtents_.x,  halfExtents_.y, -halfExtents_.z), // 2: 右上後
        Vector3(-halfExtents_.x,  halfExtents_.y, -halfExtents_.z), // 3: 左上後
        Vector3(-halfExtents_.x, -halfExtents_.y,  halfExtents_.z), // 4: 左下前
        Vector3(halfExtents_.x, -halfExtents_.y,  halfExtents_.z), // 5: 右下前
        Vector3(halfExtents_.x,  halfExtents_.y,  halfExtents_.z), // 6: 右上前
        Vector3(-halfExtents_.x,  halfExtents_.y,  halfExtents_.z)  // 7: 左上前
    };

    // ワールド空間に変換（回転して中心に配置）
    for (int i = 0; i < 8; i++) {
        corners[i] = center + Transform(localCorners[i], rotMat);
    }

    return corners;
}

Vector3 OBBCollider::GetCenter() const
{
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 affine = MakeAffineMatrix(transform.scale_, transform.rotate_, { 0,0,0 });

    Vector3 pivot = Transform(localPivot_, affine);

    return transform.transform_ + pivot;

}

bool CapsuleCollider::Contains(const Vector3& _point) const
{
    Vector3 start, end;
    GetCapsuleSegment(start, end);

    // 点から中心線への最近接点を見つける
    Vector3 closest = ClosestPointOnSegment(_point, start, end);

    // 点と最近接点の距離が半径以下なら内部に含まれる
    float distance = Vector3(_point - closest).Length();
    return distance <= radius_;
}

Vector3 CapsuleCollider::GetClosestPoint(const Vector3& _point) const
{
    Vector3 start, end;
    GetCapsuleSegment(start, end);

    // 点から中心線への最近接点を見つける
    Vector3 closest = ClosestPointOnSegment(_point, start, end);

    // 点と最近接点間のベクトル
    Vector3 toPoint = _point - closest;
    float distance = toPoint.Length();

    if (distance < 0.0001f) {
        // 点が中心線上にある場合、ランダムな方向に半径分移動
        // 簡単のため、上方向（または中心線に垂直な方向）を選択
        Vector3 worldUp = Vector3(0, 1, 0);
        Vector3 worldDirection = Transform(direction_, GetWorldTransform()->quaternion_.ToMatrix());

        // 方向ベクトルと上ベクトルが平行に近い場合は別の方向を選択
        if (std::abs(worldDirection.Dot(worldUp)) > 0.9f) {
            worldUp = Vector3(1, 0, 0); // X軸方向を使用
        }

        // 中心線に垂直なベクトルを計算
        Vector3 perpendicular = worldUp.Cross(worldDirection).Normalize();
        if (perpendicular.Length() < 0.0001f) {
            perpendicular = Vector3(0, 0, 1).Cross(worldDirection).Normalize();
        }

        return closest + perpendicular * radius_;
    }

    if (distance <= radius_) {
        // 点がカプセル内部にある場合
        return _point;
    }

    // 点がカプセル外部にある場合、最近接点から半径方向に移動
    return closest + toPoint.Normalize() * radius_;
}

Vector3 CapsuleCollider::GetCenter() const
{
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 affine = MakeAffineMatrix(transform.scale_, transform.rotate_, { 0,0,0 });

    Vector3 pivot = Transform(localPivot_, affine);

    return transform.transform_ + pivot;
}

void CapsuleCollider::GetCapsuleSegment(Vector3& _start, Vector3& _end) const
{
    Vector3 center = GetCenter();

    // トランスフォームの回転を方向ベクトルに適用
    Vector3 worldDirection = Transform(direction_, GetWorldTransform()->quaternion_.ToMatrix());

    // カプセルの中央から両端点への距離
    // 注意: カプセルの高さは両端の半球を含む全長なので、
    // 中心線の長さは (height - 2*radius) になる
    float halfLineLength = (height_ - 2.0f * radius_) * 0.5f;

    // 負の方向への端点と正の方向への端点
    _start = center - worldDirection * halfLineLength;
    _end = center + worldDirection * halfLineLength;
}

Vector3 CapsuleCollider::ClosestPointOnSegment(const Vector3& _point, const Vector3& _start, const Vector3& _end) const
{
    Vector3 segment = _end - _start;
    float segmentLength = segment.Length();

    if (segmentLength < 0.0001f) {
        return _start; // 線分の長さがほぼ0の場合
    }

    Vector3 direction = segment / segmentLength;
    float projection = Vector3(_point - _start).Dot(direction);

    // 線分上の位置にクランプ
    projection = std::clamp(projection, 0.0f, segmentLength);

    return _start + direction * projection;
}
