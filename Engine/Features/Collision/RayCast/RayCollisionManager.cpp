
#include "RayCollisionManager.h"

#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

RayCollisionManager* RayCollisionManager::GetInstance()
{
    static RayCollisionManager instance;
    return &instance;
}

bool RayCollisionManager::RayCast(const Ray& _ray, Collider* _other, RayCastHit& _hit)
{
    if (_other == nullptr)
    {
        return false;
    }
    switch (_other->GetBoundingBox())
    {
    case BoundingBox::Sphere_3D:
        return RayCastSphere(_ray, static_cast<SphereCollider*>(_other), _hit);
    case BoundingBox::AABB_3D:
        return RayCastAABB(_ray, static_cast<AABBCollider*>(_other), _hit);
    case BoundingBox::OBB_3D:
        return RayCastOBB(_ray, static_cast<OBBCollider*>(_other), _hit);
    case BoundingBox::Capsule_3D:
        return RayCastCapsule(_ray, static_cast<CapsuleCollider*>(_other), _hit);
    default:
        return false;
    }

    return _hit.hit;

}



void RayCollisionManager::RayCastAll(const Ray& _ray, std::vector<RayCastHit>& _hits, uint32_t _layerMask)
{
    for (auto collider : colliders_)
    {
        if (collider == nullptr)
        {
            continue;
        }
        // レイとコライダーの衝突判定
        RayCastHit hit;
        if (collider->GetLayer() & _layerMask)
        {
            switch (collider->GetBoundingBox())
            {
            case BoundingBox::Sphere_3D:
                RayCastSphere(_ray, static_cast<SphereCollider*>(collider), hit);
                break;
            case BoundingBox::AABB_3D:
                RayCastAABB(_ray, static_cast<AABBCollider*>(collider), hit);
                break;
            case BoundingBox::OBB_3D:
                RayCastOBB(_ray, static_cast<OBBCollider*>(collider), hit);
                break;
            case BoundingBox::Capsule_3D:
                RayCastCapsule(_ray, static_cast<CapsuleCollider*>(collider), hit);
                break;
            default:
                break;
            }
            if (hit.hit && hit.distance <= _ray.GetLength())
            {
                _hits.push_back(hit);
            }
        }
    }

    // 距離でソート
    std::sort(_hits.begin(), _hits.end(), [](const RayCastHit& _a, const RayCastHit& _b) {
        return _a.distance < _b.distance;
        });

    ClearColliders();
}

void RayCollisionManager::RegisterCollider(Collider* _collider)
{
    if (_collider == nullptr)
    {
        return;
    }

    // 重複登録を避ける
    auto it = std::find(colliders_.begin(), colliders_.end(), _collider);
    if (it == colliders_.end())
    {
        colliders_.push_back(_collider);
    }

}

bool RayCollisionManager::RayCastSphere(const Ray& _ray, SphereCollider* _collider, RayCastHit& _hit)
{
    Vector3 sphereCenter = _collider->GetWorldTransform()->GetWorldPosition();
    float sphereRadius = _collider->GetRadius();

    // レイの原点から球の中心へのベクトル
    Vector3 m = _ray.GetOrigin() - sphereCenter;

    float b = m.Dot(_ray.GetDirection());
    float c = m.Dot(m) - sphereRadius * sphereRadius;

    float deicriminant = b * b - c;

    // レイと交差しない
    if (deicriminant < 0.0f)
        return false;

    float t = -b - std::sqrt(deicriminant);

    // tが負のときレイの原点が球の内部にある
    if (t < 0.0f)
    {
        t = -b + std::sqrt(deicriminant);

        // 両方の解が負のとき
        if (t < 0.0f)
            return false;
    }

    // レイの長さを超える
    if (t > _ray.GetLength())
        return false;

    _hit.hit = true;
    _hit.distance = t;
    _hit.point = _ray.GetPoint(t);
    _hit.normal = (_hit.point - sphereCenter).Normalize();
    _hit.collider = _collider;

    return true;
}

bool RayCollisionManager::RayCastAABB(const Ray& _ray, AABBCollider* _collider, RayCastHit& _hit)
{
    Vector3 AABBpos = _collider->GetWorldTransform()->GetWorldPosition();

    Vector3 min = AABBpos + _collider->GetMin();
    Vector3 max = AABBpos + _collider->GetMax();

    Vector3 invDir = Vector3(1.0f / _ray.GetDirection().x, 1.0f / _ray.GetDirection().y, 1.0f / _ray.GetDirection().z);

    Vector3 origin = _ray.GetOrigin();
    Vector3 diff = origin - AABBpos;

    Vector3 tminVec;
    Vector3 tmaxVec;

    tminVec = (min - origin) * invDir;
    tmaxVec = (max - origin) * invDir;

    Vector3 tmin = Vector3::Min(tminVec, tmaxVec);

    Vector3 tmax = Vector3::Max(tminVec, tmaxVec);

    float tNear = (std::max)((std::max)(tmin.x, tmin.y), tmin.z);
    float tFar = (std::min)((std::min)(tmax.x, tmax.y), tmax.z);

    // 交差していない場合
    if (tNear > tFar || tFar < 0)
        return false;

    // 衝突点までの距離
    float t = (tNear >= 0) ? tNear : tFar;

    // 衝突情報を設定
    _hit.hit = true;
    _hit.distance = t;
    _hit.point = _ray.GetPoint(t);

    // 法線を計算
    Vector3 hitLocal = _hit.point - _collider->GetWorldTransform()->transform_;
    Vector3 absDistToSides = Vector3(
        std::abs(max.x - _hit.point.x),
        std::abs(max.y - _hit.point.y),
        std::abs(max.z - _hit.point.z)
    );

    // 最も近い面に応じて法線を設定
    if (absDistToSides.x < absDistToSides.y && absDistToSides.x < absDistToSides.z)
        _hit.normal = Vector3(hitLocal.x > 0 ? 1.0f : -1.0f, 0, 0);
    else if (absDistToSides.y < absDistToSides.z)
        _hit.normal = Vector3(0, hitLocal.y > 0 ? 1.0f : -1.0f, 0);
    else
        _hit.normal = Vector3(0, 0, hitLocal.z > 0 ? 1.0f : -1.0f);

    _hit.collider = _collider;

    return true;
}

bool RayCollisionManager::RayCastOBB(const Ray& _ray, OBBCollider* _collider, RayCastHit& _hit)
{// OBBの中心と回転行列を取得
    Vector3 obbCenter = _collider->GetCenter();
    WorldTransform transform = *_collider->GetWorldTransform();
    Matrix4x4 rotMat = transform.quaternion_.ToMatrix();
    Matrix4x4 invRotMat = Inverse(rotMat);

    // OBBの半分の大きさを取得
    Vector3 halfExtents = _collider->GetHalfExtents();

    // レイの始点と方向をOBBのローカル座標系に変換
    Vector3 rayOriginLocal = Transform(_ray.GetOrigin() - obbCenter, invRotMat);
    Vector3 rayDirLocal = Transform(_ray.GetDirection(), invRotMat);

    // ローカル座標系でAABBとの交差判定を行う
    Vector3 t1 = (-halfExtents - rayOriginLocal) / rayDirLocal;
    Vector3 t2 = (halfExtents - rayOriginLocal) / rayDirLocal;

    Vector3 tmin(
        std::min(t1.x, t2.x),
        std::min(t1.y, t2.y),
        std::min(t1.z, t2.z)
    );

    Vector3 tmax(
        std::max(t1.x, t2.x),
        std::max(t1.y, t2.y),
        std::max(t1.z, t2.z)
    );

    float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);

    // 交差していない場合
    if (tNear > tFar || tFar < 0)
        return false;

    // 衝突点までの距離
    float t = (tNear >= 0) ? tNear : tFar;

    // 衝突情報を設定
    _hit.hit = true;
    _hit.distance = t;
    _hit.point = _ray.GetPoint(t);

    // 法線を計算（ローカル座標系でどの面にヒットしたかを特定）
    Vector3 hitLocalPos = rayOriginLocal + rayDirLocal * t;
    Vector3 absDistToSides = Vector3(
        std::abs(halfExtents.x - std::abs(hitLocalPos.x)),
        std::abs(halfExtents.y - std::abs(hitLocalPos.y)),
        std::abs(halfExtents.z - std::abs(hitLocalPos.z))
    );

    Vector3 localNormal;
    if (absDistToSides.x < absDistToSides.y && absDistToSides.x < absDistToSides.z)
        localNormal = Vector3(hitLocalPos.x > 0 ? 1.0f : -1.0f, 0, 0);
    else if (absDistToSides.y < absDistToSides.z)
        localNormal = Vector3(0, hitLocalPos.y > 0 ? 1.0f : -1.0f, 0);
    else
        localNormal = Vector3(0, 0, hitLocalPos.z > 0 ? 1.0f : -1.0f);

    // ローカル座標系の法線をワールド座標系に変換
    _hit.normal = Transform(localNormal, rotMat).Normalize();
    _hit.collider = _collider;

    return true;
}

bool RayCollisionManager::RayCastCapsule(const Ray& _ray, CapsuleCollider* _collider, RayCastHit& _hit)
{
    // カプセルの中心線の両端点を取得
    Vector3 capsuleStart, capsuleEnd;
    _collider->GetCapsuleSegment(capsuleStart, capsuleEnd);
    float radius = _collider->GetRadius();

    // レイと線分の最短距離を計算
    Vector3 rayOrigin = _ray.GetOrigin();
    Vector3 rayDir = _ray.GetDirection();

    Vector3 capsuleDir = capsuleEnd - capsuleStart;
    Vector3 w = rayOrigin - capsuleStart;

    float a = rayDir.Dot(rayDir);
    float b = rayDir.Dot(capsuleDir);
    float c = capsuleDir.Dot(capsuleDir);
    float d = rayDir.Dot(w);
    float e = capsuleDir.Dot(w);

    float denominator = a * c - b * b;

    // パラメータ t（レイ）と s（カプセル中心線）を計算
    float t, s;

    if (denominator < 0.0001f) {
        // ほぼ平行な場合
        s = 0.0f;
        t = d / a;
    }
    else {
        s = std::clamp((a * e - b * d) / denominator, 0.0f, 1.0f);
        t = (b * s - d) / a;
    }

    // t が負の場合、レイの始点がカプセルの後方にある
    if (t < 0) {
        // レイの始点とカプセル中心線の距離を計算
        Vector3 pointOnCapsule = capsuleStart + capsuleDir * s;
        Vector3 toRayOrigin = rayOrigin - pointOnCapsule;
        float distSq = toRayOrigin.LengthSquared();

        // 距離が半径より大きい場合、衝突なし
        if (distSq > radius * radius)
            return false;

        // レイの始点からカプセルに向かう方向に進んだ点で衝突
        float penetration = std::sqrt(radius * radius - distSq);
        t = -toRayOrigin.Dot(rayDir) - penetration;

        // それでも t が負の場合、レイは離れる方向に向かっている
        if (t < 0)
            return false;
    }

    // レイとカプセル中心線上の最近接点を計算
    Vector3 pointOnRay = rayOrigin + rayDir * t;
    Vector3 pointOnCapsule = capsuleStart + capsuleDir * s;

    // 最近接点間の距離を計算
    Vector3 closestToRay = pointOnRay - pointOnCapsule;
    float distSq = closestToRay.LengthSquared();

    // 距離が半径より大きい場合、衝突なし
    if (distSq > radius * radius)
        return false;

    // 最終的な衝突距離を計算
    float dist = t;
    if (distSq > 0.0001f) {
        float penetration = radius - std::sqrt(distSq);
        dist = t - penetration;
    }

    // 衝突情報を設定
    _hit.hit = true;
    _hit.distance = dist;
    _hit.point = rayOrigin + rayDir * dist;

    // 衝突点の法線を計算
    if (distSq > 0.0001f) {
        _hit.normal = closestToRay.Normalize();
    }
    else {
        // レイがカプセル中心線と直接交差している場合、適切な法線を計算
        Vector3 perpendicular;
        if (std::abs(rayDir.Dot(Vector3(0, 1, 0))) < 0.9f)
            perpendicular = rayDir.Cross(Vector3(0, 1, 0)).Normalize();
        else
            perpendicular = rayDir.Cross(Vector3(1, 0, 0)).Normalize();

        _hit.normal = perpendicular;
    }

    _hit.collider = _collider;

    return true;
}
