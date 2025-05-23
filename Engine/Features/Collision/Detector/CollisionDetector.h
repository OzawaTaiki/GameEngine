#pragma once

#include <Features/Collision/Collider/Collider.h>

// 衝突判定を行うクラス
class CollisionDetector
{
public:
    // 各コライダー同士の衝突判定
    static bool DetectCollision(Collider* _colliderA, Collider* _colliderB, ColliderInfo& _info);

private:
    // 球と球の衝突判定
    static bool IntersectSphereSphere(SphereCollider* _sphere1, SphereCollider* _sphere2, ColliderInfo& _info);

    // AABBとAABBの衝突判定
    static bool IntersectAABBAABB(AABBCollider* _aabb1, AABBCollider* _aabb2, ColliderInfo& _info);

    // OBBとOBBの衝突判定
    static bool IntersectOBBOBB(OBBCollider* _obb1, OBBCollider* _obb2, ColliderInfo& _info);

    // カプセルとカプセルの衝突判定
    static bool IntersectCapsuleCapsule(CapsuleCollider* _capsule1, CapsuleCollider* _capsule2, ColliderInfo& _info);

    // 球とAABBの衝突判定
    static bool IntersectSphereAABB(SphereCollider* _sphere, AABBCollider* _aabb, ColliderInfo& _info);

    // 球とOBBの衝突判定
    static bool IntersectSphereOBB(SphereCollider* _sphere, OBBCollider* _obb, ColliderInfo& _info);

    // 球とカプセルの衝突判定
    static bool IntersectSphereCapsule(SphereCollider* _sphere, CapsuleCollider* _capsule, ColliderInfo& _info);

    // AABBとOBBの衝突判定
    static bool IntersectAABBOBB(AABBCollider* _aabb, OBBCollider* _obb, ColliderInfo& _info);

    // AABBとカプセルの衝突判定
    static bool IntersectAABBCapsule(AABBCollider* _aabb, CapsuleCollider* _capsule, ColliderInfo& _info);

    // OBBとカプセルの衝突判定
    static bool IntersectOBBCapsule(OBBCollider* _obb, CapsuleCollider* _capsule, ColliderInfo& _info);

    // Y軸回転特化のAABB vs OBB判定
    static bool IntersectAABBOBB_YRotationOnly(AABBCollider* _aabb, OBBCollider* _obb, ColliderInfo& _info);

    // 2D回転矩形 vs 軸平行矩形の判定（XZ平面）
    static bool IntersectRotatedRect_vs_AxisAlignedRect(const Vector2& _rectCenter, const Vector2& _rectHalfSize, float _rotationY,
        const Vector2& _aabbMin, const Vector2& _aabbMax, Vector2& _contactPoint, Vector2& _contactNormal, float& _penetration);


    // 2つの線分間の最短距離を計算（カプセル衝突判定用）
    static float SegmentSegmentDistance(
        const Vector3& _start1, const Vector3& _end1,
        const Vector3& _start2, const Vector3& _end2,
        Vector3& _closestPoint1, Vector3& _closestPoint2);

    static bool CheckBoundingSpheres(Collider* _colliderA, Collider* _colliderB, float expansionFactor);


    // 球の半径を取得
    static float GetBoundingSphereRadius(Collider* _collider);

};