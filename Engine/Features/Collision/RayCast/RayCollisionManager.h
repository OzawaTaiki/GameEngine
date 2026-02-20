#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/RayCast/Ray.h>

// レイとの衝突結果

namespace Engine {

struct RayCastHit
{
    bool hit = false;            // 衝突したかどうか
    float distance = 0.0f;       // 衝突点までの距離
    Vector3 point;              // 衝突点
    Vector3 normal;             // 衝突点での法線
    Collider* collider = nullptr; // 衝突したコライダー
};


class RayCollisionManager
{
public:

    // インスタンスを取得する
    static RayCollisionManager* GetInstance();

    bool RayCast(const Ray& _ray, Collider* _other, RayCastHit& _hit);

    // 全コライダーとの衝突判定
    void RayCastAll(const Ray& _ray, std::vector<RayCastHit>& _hits, uint32_t _layerMask);

    void ClearColliders() { colliders_.clear(); }

    void RegisterCollider(Collider* _collider);


private:

    std::vector<Collider*> colliders_; // コライダーのリスト

    // 各コライダーとの衝突判定
    bool RayCastSphere(const Ray& _ray, SphereCollider* _collider, RayCastHit& _hit);

    bool RayCastAABB(const Ray& _ray, AABBCollider* _collider, RayCastHit& _hit);

    bool RayCastOBB(const Ray& _ray, OBBCollider* _collider, RayCastHit& _hit);

    bool RayCastCapsule(const Ray& _ray, CapsuleCollider* _collider, RayCastHit& _hit);


};

} // namespace Engine
