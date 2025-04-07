#define NOMINMAX
#include "CollisionDetector.h"
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>


bool CollisionDetector::DetectCollision(Collider* _colliderA, Collider* _colliderB, ColliderInfo& _info)
{
    // nullチェック
    if (_colliderA == nullptr || _colliderB == nullptr)
        return false;

    // バウンディングボックスの種類を取得
    BoundingBox typeA = _colliderA->GetBoundingBox();
    BoundingBox typeB = _colliderB->GetBoundingBox();

    // 衝突判定を実行
    // 各バウンディングボックスの組み合わせに対して適切な判定処理を呼び出す

    // Sphere vs Sphere
    if (typeA == BoundingBox::Sphere_3D && typeB == BoundingBox::Sphere_3D)
    {
        return IntersectSphereSphere(
            static_cast<SphereCollider*>(_colliderA),
            static_cast<SphereCollider*>(_colliderB),
            _info
        );
    }

    // AABB vs AABB
    else if (typeA == BoundingBox::AABB_3D && typeB == BoundingBox::AABB_3D)
    {
        return IntersectAABBAABB(
            static_cast<AABBCollider*>(_colliderA),
            static_cast<AABBCollider*>(_colliderB),
            _info
        );
    }

    // OBB vs OBB
    else if (typeA == BoundingBox::OBB_3D && typeB == BoundingBox::OBB_3D)
    {
        return IntersectOBBOBB(
            static_cast<OBBCollider*>(_colliderA),
            static_cast<OBBCollider*>(_colliderB),
            _info
        );
    }

    // Capsule vs Capsule
    else if (typeA == BoundingBox::Capsule_3D && typeB == BoundingBox::Capsule_3D)
    {
        return IntersectCapsuleCapsule(
            static_cast<CapsuleCollider*>(_colliderA),
            static_cast<CapsuleCollider*>(_colliderB),
            _info
        );
    }

    // Sphere vs AABB
    else if (typeA == BoundingBox::Sphere_3D && typeB == BoundingBox::AABB_3D)
    {
        return IntersectSphereAABB(
            static_cast<SphereCollider*>(_colliderA),
            static_cast<AABBCollider*>(_colliderB),
            _info
        );
    }

    // AABB vs Sphere
    else if (typeA == BoundingBox::AABB_3D && typeB == BoundingBox::Sphere_3D)
    {
        bool result = IntersectSphereAABB(
            static_cast<SphereCollider*>(_colliderB),
            static_cast<AABBCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // Sphere vs OBB
    else if (typeA == BoundingBox::Sphere_3D && typeB == BoundingBox::OBB_3D)
    {
        return IntersectSphereOBB(
            static_cast<SphereCollider*>(_colliderA),
            static_cast<OBBCollider*>(_colliderB),
            _info
        );
    }

    // OBB vs Sphere
    else if (typeA == BoundingBox::OBB_3D && typeB == BoundingBox::Sphere_3D)
    {
        bool result = IntersectSphereOBB(
            static_cast<SphereCollider*>(_colliderB),
            static_cast<OBBCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // Sphere vs Capsule
    else if (typeA == BoundingBox::Sphere_3D && typeB == BoundingBox::Capsule_3D)
    {
        return IntersectSphereCapsule(
            static_cast<SphereCollider*>(_colliderA),
            static_cast<CapsuleCollider*>(_colliderB),
            _info
        );
    }

    // Capsule vs Sphere
    else if (typeA == BoundingBox::Capsule_3D && typeB == BoundingBox::Sphere_3D)
    {
        bool result = IntersectSphereCapsule(
            static_cast<SphereCollider*>(_colliderB),
            static_cast<CapsuleCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // AABB vs OBB
    else if (typeA == BoundingBox::AABB_3D && typeB == BoundingBox::OBB_3D)
    {
        return IntersectAABBOBB(
            static_cast<AABBCollider*>(_colliderA),
            static_cast<OBBCollider*>(_colliderB),
            _info
        );
    }

    // OBB vs AABB
    else if (typeA == BoundingBox::OBB_3D && typeB == BoundingBox::AABB_3D)
    {
        bool result = IntersectAABBOBB(
            static_cast<AABBCollider*>(_colliderB),
            static_cast<OBBCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // AABB vs Capsule
    else if (typeA == BoundingBox::AABB_3D && typeB == BoundingBox::Capsule_3D)
    {
        return IntersectAABBCapsule(
            static_cast<AABBCollider*>(_colliderA),
            static_cast<CapsuleCollider*>(_colliderB),
            _info
        );
    }

    // Capsule vs AABB
    else if (typeA == BoundingBox::Capsule_3D && typeB == BoundingBox::AABB_3D)
    {
        bool result = IntersectAABBCapsule(
            static_cast<AABBCollider*>(_colliderB),
            static_cast<CapsuleCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // OBB vs Capsule
    else if (typeA == BoundingBox::OBB_3D && typeB == BoundingBox::Capsule_3D)
    {
        return IntersectOBBCapsule(
            static_cast<OBBCollider*>(_colliderA),
            static_cast<CapsuleCollider*>(_colliderB),
            _info
        );
    }

    // Capsule vs OBB
    else if (typeA == BoundingBox::Capsule_3D && typeB == BoundingBox::OBB_3D)
    {
        bool result = IntersectOBBCapsule(
            static_cast<OBBCollider*>(_colliderB),
            static_cast<CapsuleCollider*>(_colliderA),
            _info
        );

        // 法線方向を反転
        if (result)
        {
            _info.contactNormal = -_info.contactNormal;
        }

        return result;
    }

    // サポートされていない組み合わせ
    return false;
}

bool CollisionDetector::IntersectSphereSphere(SphereCollider* _sphere1, SphereCollider* _sphere2, ColliderInfo& _info)
{
    // 球の中心位置を取得
    Vector3 center1 = _sphere1->GetWorldTransform()->GetWorldPosition() + _sphere1->GetOffset();
    Vector3 center2 = _sphere2->GetWorldTransform()->GetWorldPosition() + _sphere2->GetOffset();

    // 2つの球の中心間の距離を計算
    Vector3 direction = center2 - center1;
    float distance = direction.Length();
    float radiusSum = _sphere1->GetRadius() + _sphere2->GetRadius();

    // 距離が半径の和より大きい場合は衝突していない
    if (distance * distance > radiusSum * radiusSum)
    {
        return false;
    }

    // 衝突情報を設定
    _info.hasCollision = true;

    // 方向ベクトルの正規化（ゼロベクトルの場合は適当な方向を設定）
    if (distance > 0.0001f)
    {
        _info.contactNormal = direction / distance;
    }
    else
    {
        _info.contactNormal = Vector3(0, 1, 0); // 上向きの適当な法線
    }

    // めり込み量
    _info.penetration = radiusSum - distance;

    // 衝突点（球1の中心から球2方向へ球1の半径分移動した点）
    _info.contactPoint = center1 + _info.contactNormal * _sphere1->GetRadius();

    return true;
}

bool CollisionDetector::IntersectAABBAABB(AABBCollider* _aabb1, AABBCollider* _aabb2, ColliderInfo& _info)
{
    // AABBのワールド座標を計算
    const WorldTransform* transform1 = _aabb1->GetWorldTransform();
    const WorldTransform* transform2 = _aabb2->GetWorldTransform();

    Vector3 min1 = _aabb1->GetMin() * transform1->scale_ + transform1->transform_ + _aabb1->GetOffset();
    Vector3 max1 = _aabb1->GetMax() * transform1->scale_ + transform1->transform_ + _aabb1->GetOffset();
    Vector3 min2 = _aabb2->GetMin() * transform2->scale_ + transform2->transform_;
    Vector3 max2 = _aabb2->GetMax() * transform2->scale_ + transform2->transform_;

    // 各軸で重なりを確認
    if (max1.x < min2.x || min1.x > max2.x ||
        max1.y < min2.y || min1.y > max2.y ||
        max1.z < min2.z || min1.z > max2.z)
    {
        return false;
    }

    // 衝突情報を設定
    _info.hasCollision = true;

    // めり込みが最も小さい軸を見つける
    float overlapX = std::min(max1.x, max2.x) - std::max(min1.x, min2.x);
    float overlapY = std::min(max1.y, max2.y) - std::max(min1.y, min2.y);
    float overlapZ = std::min(max1.z, max2.z) - std::max(min1.z, min2.z);

    // めり込みが最小の軸に沿った法線と衝突点を計算
    if (overlapX <= overlapY && overlapX <= overlapZ)
    {
        // X軸でのめり込みが最小
        _info.penetration = overlapX;
        _info.contactNormal = (transform1->transform_.x < transform2->transform_.x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
    }
    else if (overlapY <= overlapX && overlapY <= overlapZ)
    {
        // Y軸でのめり込みが最小
        _info.penetration = overlapY;
        _info.contactNormal = (transform1->transform_.y < transform2->transform_.y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
    }
    else
    {
        // Z軸でのめり込みが最小
        _info.penetration = overlapZ;
        _info.contactNormal = (transform1->transform_.z < transform2->transform_.z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
    }

    // 衝突点を計算（2つのAABBの中心からめり込み方向に半分移動した点）
    Vector3 center1 = (min1 + max1) * 0.5f;
    Vector3 center2 = (min2 + max2) * 0.5f;
    _info.contactPoint = center1 - _info.contactNormal * (_info.penetration * 0.5f);

    return true;
}

bool CollisionDetector::IntersectOBBOBB(OBBCollider* _obb1, OBBCollider* _obb2, ColliderInfo& _info)
{
    WorldTransform transform1 = *_obb1->GetWorldTransform();
    WorldTransform transform2 = *_obb2->GetWorldTransform();

    Vector3 scale1 = transform1.scale_;
    Vector3 scale2 = transform2.scale_;


    // 分離軸テスト
    const Vector3 center1 = _obb1->GetCenter();
    const Vector3 center2 = _obb2->GetCenter();
    const Vector3 halfExtents1 = _obb1->GetHalfExtents() * scale1;
    const Vector3 halfExtents2 = _obb2->GetHalfExtents() * scale2;

    // OBB1の軸
    Matrix4x4 rotMat1 = _obb1->GetWorldTransform()->quaternion_.ToMatrix();
    Vector3 axis1[3] = {
        Transform(Vector3(1, 0, 0), rotMat1),
        Transform(Vector3(0, 1, 0), rotMat1),
        Transform(Vector3(0, 0, 1), rotMat1)
    };

    // OBB2の軸
    Matrix4x4 rotMat2 = _obb2->GetWorldTransform()->quaternion_.ToMatrix();
    Vector3 axis2[3] = {
        Transform(Vector3(1, 0, 0), rotMat2),
        Transform(Vector3(0, 1, 0), rotMat2),
        Transform(Vector3(0, 0, 1), rotMat2)
    };

    // 中心間のベクトル
    Vector3 T = center2 - center1;

    // 最小の重なり量とその軸を追跡
    float minOverlap = FLT_MAX;
    Vector3 bestAxis;

    // OBB1の3つの軸についてテスト
    for (int i = 0; i < 3; i++)
    {
        Vector3 axis = axis1[i];

        float ra = halfExtents1[i];
        float rb = halfExtents2[0] * std::abs(axis.Dot(axis2[0])) +
            halfExtents2[1] * std::abs(axis.Dot(axis2[1])) +
            halfExtents2[2] * std::abs(axis.Dot(axis2[2]));

        float t = std::abs(T.Dot(axis));

        // 分離軸が見つかった
        if (t > ra + rb)
        {
            return false;
        }

        // めり込みを追跡
        float overlap = ra + rb - t;
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (T.Dot(axis) < 0) bestAxis = -bestAxis; // 方向を適切に
        }
    }

    // OBB2の3つの軸についてテスト
    for (int i = 0; i < 3; i++)
    {
        Vector3 axis = axis2[i];

        float ra = halfExtents1[0] * std::abs(axis.Dot(axis1[0])) +
            halfExtents1[1] * std::abs(axis.Dot(axis1[1])) +
            halfExtents1[2] * std::abs(axis.Dot(axis1[2]));
        float rb = halfExtents2[i];

        float t = std::abs(T.Dot(axis));

        // 分離軸が見つかった
        if (t > ra + rb)
        {
            return false;
        }

        // めり込みを追跡
        float overlap = ra + rb - t;
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (T.Dot(axis) < 0) bestAxis = -bestAxis; // 方向を適切に
        }
    }

    // 交差積の9つの軸についてテスト
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            Vector3 axis = axis1[i].Cross(axis2[j]);

            // ほぼ平行な軸は無視
            float length = axis.Length();
            if (length * length < 0.0001f)
                continue;

            axis = axis.Normalize();

            float ra = halfExtents1[0] * std::abs(axis.Dot(axis1[0])) +
                halfExtents1[1] * std::abs(axis.Dot(axis1[1])) +
                halfExtents1[2] * std::abs(axis.Dot(axis1[2]));
            float rb = halfExtents2[0] * std::abs(axis.Dot(axis2[0])) +
                halfExtents2[1] * std::abs(axis.Dot(axis2[1])) +
                halfExtents2[2] * std::abs(axis.Dot(axis2[2]));

            float t = std::abs(T.Dot(axis));

            // 分離軸が見つかった
            if (t > ra + rb)
            {
                return false;
            }

            // めり込みを追跡
            float overlap = ra + rb - t;
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                bestAxis = axis;
                if (T.Dot(axis) < 0) bestAxis = -bestAxis; // 方向を適切に
            }
        }
    }

    // すべての軸でテストをパスした場合、衝突している
    _info.hasCollision = true;
    _info.contactNormal = bestAxis;
    _info.penetration = minOverlap;

    // 衝突点を近似的に求める（OBB1とOBB2の中間点）
    _info.contactPoint = center1 + T * 0.5f;

    return true;
}

bool CollisionDetector::IntersectCapsuleCapsule(CapsuleCollider* _capsule1, CapsuleCollider* _capsule2, ColliderInfo& _info)
{
    // カプセルの線分端点を取得
    Vector3 start1, end1, start2, end2;
    _capsule1->GetCapsuleSegment(start1, end1);
    _capsule2->GetCapsuleSegment(start2, end2);

    // 線分間の最短距離を計算
    Vector3 closestPoint1, closestPoint2;
    float segmentDistance = SegmentSegmentDistance(
        start1, end1, start2, end2, closestPoint1, closestPoint2);

    float radiusSum = _capsule1->GetRadius() + _capsule2->GetRadius();

    // 衝突判定
    if (segmentDistance >= radiusSum)
    {
        return false;
    }

    // 衝突情報を設定
    _info.hasCollision = true;

    // 接触法線
    Vector3 normal = closestPoint2 - closestPoint1;
    float length = normal.Length();

    if (length > 0.0001f)
    {
        _info.contactNormal = normal / length;
    }
    else
    {
        // 中心線が一致した場合は適当な方向を設定
        _info.contactNormal = Vector3(0, 1, 0);
    }

    _info.penetration = radiusSum - length;
    _info.contactPoint = closestPoint1 + _info.contactNormal * _capsule1->GetRadius();

    return true;
}

bool CollisionDetector::IntersectSphereAABB(SphereCollider* _sphere, AABBCollider* _aabb, ColliderInfo& _info)
{
    // 球の中心
    Vector3 sphereCenter = _sphere->GetWorldTransform()->transform_ + _sphere->GetOffset();
    float radius = _sphere->GetRadius();

    // AABBのワールド座標
    const WorldTransform* aabbTransform = _aabb->GetWorldTransform();
    Vector3 aabbMin = _aabb->GetMin() + aabbTransform->transform_ + _aabb->GetOffset();
    Vector3 aabbMax = _aabb->GetMax() + aabbTransform->transform_ + _aabb->GetOffset();

    // 球の中心からAABBへの最近接点を計算
    Vector3 closestPoint(
        std::clamp(sphereCenter.x, aabbMin.x, aabbMax.x),
        std::clamp(sphereCenter.y, aabbMin.y, aabbMax.y),
        std::clamp(sphereCenter.z, aabbMin.z, aabbMax.z)
    );

    // 球の中心と最近接点の距離を計算
    Vector3 delta = sphereCenter - closestPoint;
    float length = delta.Length();
    float distanceSquared = length * length;

    // 距離が半径より大きい場合は衝突していない
    if (distanceSquared > radius * radius)
    {
        return false;
    }

    float distance = length;

    // 衝突情報を設定
    _info.hasCollision = true;

    if (distance > 0.0001f)
    {
        _info.contactNormal = delta / distance;
    }
    else
    {
        // 球の中心がAABB内部にある場合、最も近い面の法線を使用
        Vector3 center = (aabbMin + aabbMax) * 0.5f;
        Vector3 extents = aabbMax - center;

        Vector3 diff = sphereCenter - center;
        Vector3 absDiff(std::abs(diff.x), std::abs(diff.y), std::abs(diff.z));
        Vector3 relDiff = absDiff / extents;

        if (relDiff.x > relDiff.y && relDiff.x > relDiff.z)
        {
            _info.contactNormal = Vector3(diff.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
        }
        else if (relDiff.y > relDiff.x && relDiff.y > relDiff.z)
        {
            _info.contactNormal = Vector3(0.0f, diff.y > 0.0f ? 1.0f : -1.0f, 0.0f);
        }
        else
        {
            _info.contactNormal = Vector3(0.0f, 0.0f, diff.z > 0.0f ? 1.0f : -1.0f);
        }
    }

    _info.penetration = radius - distance;
    _info.contactPoint = closestPoint;

    return true;
}

bool CollisionDetector::IntersectSphereOBB(SphereCollider* _sphere, OBBCollider* _obb, ColliderInfo& _info)
{
    // 球の中心
    Vector3 sphereCenter = _sphere->GetWorldTransform()->transform_ + _sphere->GetOffset();
    float radius = _sphere->GetRadius();

    // OBBの最近接点を計算
    Vector3 closestPoint = _obb->GetClosestPoint(sphereCenter);

    // 球の中心と最近接点の距離を計算
    Vector3 delta = sphereCenter - closestPoint;
    float length = delta.Length();
    float distanceSquared = length * length;

    // 距離が半径より大きい場合は衝突していない
    if (distanceSquared > radius * radius)
    {
        return false;
    }

    float distance = std::sqrt(distanceSquared);

    // 衝突情報を設定
    _info.hasCollision = true;

    if (distance > 0.0001f)
    {
        _info.contactNormal = delta / distance;
    }
    else
    {
        // 球の中心がOBB内部にある場合、適当な法線を設定
        _info.contactNormal = Vector3(0, 1, 0);
    }

    _info.penetration = radius - distance;
    _info.contactPoint = closestPoint;

    return true;
}

bool CollisionDetector::IntersectSphereCapsule(SphereCollider* _sphere, CapsuleCollider* _capsule, ColliderInfo& _info)
{
    // 球の中心
    Vector3 sphereCenter = _sphere->GetWorldTransform()->transform_ + _sphere->GetOffset();
    float sphereRadius = _sphere->GetRadius();

    // カプセルの線分端点を取得
    Vector3 start, end;
    _capsule->GetCapsuleSegment(start, end);

    // 線分上の最近接点を計算
    Vector3 closestPoint = _capsule->ClosestPointOnSegment(sphereCenter, start, end);

    // 球の中心と最近接点の距離を計算
    Vector3 delta = sphereCenter - closestPoint;
    float distanceSquared = delta.LengthSquared();

    float radiusSum = sphereRadius + _capsule->GetRadius();

    // 距離が半径の和より大きい場合は衝突していない
    if (distanceSquared > radiusSum * radiusSum)
    {
        return false;
    }

    float distance = std::sqrt(distanceSquared);

    // 衝突情報を設定
    _info.hasCollision = true;

    if (distance > 0.0001f)
    {
        _info.contactNormal = delta / distance;
    }
    else
    {
        // 球の中心がカプセル内部にある場合、適当な法線を設定
        _info.contactNormal = Vector3(0, 1, 0);
    }

    _info.penetration = radiusSum - distance;
    _info.contactPoint = closestPoint + _info.contactNormal * _capsule->GetRadius();

    return true;
}

bool CollisionDetector::IntersectAABBOBB(AABBCollider* _aabb, OBBCollider* _obb, ColliderInfo& _info)
{
    // AABBをOBBとして扱って、OBB vs OBB の判定を利用
    OBBCollider tempOBB;
    tempOBB.SetBoundingBox(BoundingBox::OBB_3D);

    // AABBの中心と半分のサイズを計算
    Vector3 aabbMin = _aabb->GetMin();
    Vector3 aabbMax = _aabb->GetMax();
    Vector3 center = (aabbMin + aabbMax) * 0.5f;
    Vector3 halfExtents = (aabbMax - aabbMin) * 0.5f;

    // 一時的なOBBを設定
    tempOBB.SetHalfExtents(halfExtents);
    tempOBB.SetLocalPivot(center);
    tempOBB.SetWorldTransform(_aabb->GetWorldTransform());

    // OBB vs OBB の判定を利用
    return IntersectOBBOBB(&tempOBB, _obb, _info);
}

bool CollisionDetector::IntersectAABBCapsule(AABBCollider* _aabb, CapsuleCollider* _capsule, ColliderInfo& _info)
{
    // カプセルの線分端点を取得
    Vector3 start, end;
    _capsule->GetCapsuleSegment(start, end);

    // AABBのワールド座標
    const WorldTransform* aabbTransform = _aabb->GetWorldTransform();
    Vector3 aabbMin = _aabb->GetMin() + aabbTransform->transform_ + _aabb->GetOffset();
    Vector3 aabbMax = _aabb->GetMax() + aabbTransform->transform_ + _aabb->GetOffset();

    // 線分とAABBの最短距離を計算
    float minDistance = FLT_MAX;
    Vector3 closestPoint;

    // 線分の各点に対して最短距離を計算
    for (float t = 0; t <= 1.0f; t += 0.1f)
    {
        Vector3 point = start + (end - start) * t;
        Vector3 closest(
            std::clamp(point.x, aabbMin.x, aabbMax.x),
            std::clamp(point.y, aabbMin.y, aabbMax.y),
            std::clamp(point.z, aabbMin.z, aabbMax.z)
        );

        float distSq = (point - closest).LengthSquared();
        if (distSq < minDistance)
        {
            minDistance = distSq;
            closestPoint = closest;
        }
    }

    minDistance = std::sqrt(minDistance);

    // 距離がカプセルの半径より大きい場合は衝突していない
    if (minDistance > _capsule->GetRadius())
    {
        return false;
    }

    // 衝突情報を設定
    _info.hasCollision = true;

    // 衝突点に最も近いカプセル線分上の点を見つける
    Vector3 linePoint = _capsule->ClosestPointOnSegment(closestPoint, start, end);
    Vector3 delta = closestPoint - linePoint;
    float length = delta.Length();

    if (length > 0.0001f)
    {
        _info.contactNormal = delta / length;
    }
    else
    {
        // 適当な法線を設定
        _info.contactNormal = Vector3(0, 1, 0);
    }

    _info.penetration = _capsule->GetRadius() - length;
    _info.contactPoint = closestPoint;

    return true;
}

bool CollisionDetector::IntersectOBBCapsule(OBBCollider* _obb, CapsuleCollider* _capsule, ColliderInfo& _info)
{
    // カプセルの線分端点を取得
    Vector3 start, end;
    _capsule->GetCapsuleSegment(start, end);

    // 線分とOBBの最短距離を計算
    float minDistance = FLT_MAX;
    Vector3 closestPoint;

    // 線分の各点に対して最短距離を計算
    for (float t = 0; t <= 1.0f; t += 0.1f)
    {
        Vector3 point = start + (end - start) * t;
        Vector3 closest = _obb->GetClosestPoint(point);

        float distSq = (point - closest).LengthSquared();
        if (distSq < minDistance)
        {
            minDistance = distSq;
            closestPoint = closest;
        }
    }

    minDistance = std::sqrt(minDistance);

    // 距離がカプセルの半径より大きい場合は衝突していない
    if (minDistance > _capsule->GetRadius())
    {
        return false;
    }

    // 衝突情報を設定
    _info.hasCollision = true;

    // 衝突点に最も近いカプセル線分上の点を見つける
    Vector3 linePoint = _capsule->ClosestPointOnSegment(closestPoint, start, end);
    Vector3 delta = closestPoint - linePoint;
    float length = delta.Length();

    if (length > 0.0001f)
    {
        _info.contactNormal = delta / length;
    }
    else
    {
        // 適当な法線を設定
        _info.contactNormal = Vector3(0, 1, 0);
    }

    _info.penetration = _capsule->GetRadius() - length;
    _info.contactPoint = closestPoint;

    return true;
}

float CollisionDetector::SegmentSegmentDistance(const Vector3& _start1, const Vector3& _end1, const Vector3& _start2, const Vector3& _end2, Vector3& _closestPoint1, Vector3& _closestPoint2)
{
    Vector3 d1 = _end1 - _start1;
    Vector3 d2 = _end2 - _start2;
    Vector3 r = _start1 - _start2;

    float a = d1.LengthSquared();
    float e = d2.LengthSquared();
    float f = d2.Dot(r);

    // パラメータが範囲外の場合の処理のための変数
    float s = 0.0f;
    float t = 0.0f;

    // 両方の線分が点に縮退している場合
    if (a <= 0.0001f && e <= 0.0001f)
    {
        _closestPoint1 = _start1;
        _closestPoint2 = _start2;
        return (_closestPoint1 - _closestPoint2).Length();
    }

    // 線分1が点に縮退している場合
    if (a <= 0.0001f)
    {
        s = 0.0f;
        t = f / e;
        t = std::clamp(t, 0.0f, 1.0f);
    }
    else
    {
        float c = d1.Dot(r);

        // 線分2が点に縮退している場合
        if (e <= 0.0001f)
        {
            t = 0.0f;
            s = std::clamp(-c / a, 0.0f, 1.0f);
        }
        else
        {
            // 一般的な場合
            float b = d1.Dot(d2);
            float denom = a * e - b * b;

            // 線分が平行でない場合
            if (denom != 0.0f)
            {
                s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }
            else
            {
                s = 0.0f;
            }

            // 線分2上のパラメータを計算
            t = (b * s + f) / e;

            // tを[0,1]の範囲にクランプし、sを再計算
            if (t < 0.0f)
            {
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = std::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    // 最近接点を計算
    _closestPoint1 = _start1 + d1 * s;
    _closestPoint2 = _start2 + d2 * t;

    // 距離を返す
    return (_closestPoint1 - _closestPoint2).Length();
}
