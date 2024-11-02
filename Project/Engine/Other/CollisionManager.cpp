#define NOMINMAX
#include "CollisionManager.h"
#include "Collider.h"
#include "MyLib.h"
#include "ImGuiManager.h"
#include <algorithm>

CollisionManager* CollisionManager::GetInstance()
{
    static CollisionManager instance;
    return &instance;
}

void CollisionManager::SetCollider(Collider* _collider)
{
    colliders_.push_back(_collider);
}

void CollisionManager::CheckAllCollisions()
{
    for (auto itA = colliders_.begin(); itA != colliders_.end(); itA++)
    {
        for (auto itB = std::next(itA); itB != colliders_.end(); itB++)
        {
            if ((*itA)->GetMask() & (*itB)->GetAtrribute_() ||
                (*itB)->GetMask() & (*itA)->GetAtrribute_())
            {
                CheckCollisionPair(*itA, *itB);
            }
        }
    }
}

uint32_t CollisionManager::GetAtttibute(const std::string& _atrribute)
{
    auto it = map_.find(_atrribute);
    if (it != map_.end())
        return it->second;

    map_[_atrribute] = static_cast <uint32_t>(1 << map_.size());
    uint32_t result = map_[_atrribute];
    return result;
}

uint32_t CollisionManager::GetMask(const std::string& _atrribute)
{
    return ~GetAtttibute(_atrribute);
}

void CollisionManager::CheckCollisionPair(Collider* _colliderA, Collider* _colliderB)
{
    Vector3 wPosA = _colliderA->GetWorldPosition();
    Vector3 wPosB = _colliderB->GetWorldPosition();

     switch (_colliderA->GetBoundingBox())
    {
    case Collider::BoundingBox::Sphere_3D:
        {

            Sphere colA = {
                .center = wPosA,
                .radius = _colliderA->GetRadius()
            };
            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:
                {
                    Sphere colB = {
                   .center = wPosB,
                   .radius = _colliderB->GetRadius()
                    };

                    if (IsCollision(colA,colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            case Collider::BoundingBox::AABB_3D:
                {
                    AABB colB = {
                        .center = wPosB,
                        .min = -_colliderA->GetSize() / 2.0f,
                        .max = _colliderA->GetSize() / 2.0f,
                    };

                    if (IsCollision(colA, colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            case Collider::BoundingBox::OBB_3D:
                {
                    OBB colB = {
                        .center = wPosB,
                        .size = _colliderB->GetSize(),
                        .rotate = _colliderB->GetRotate()
                    };
                    colB.CalculateOrientations();
                    if (IsCollision(colA, colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            default:
                break;
            }
        }
        break;
    case Collider::BoundingBox::AABB_3D:
        {
            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:

                break;
            case Collider::BoundingBox::AABB_3D:
                break;
            case Collider::BoundingBox::OBB_3D:
                break;
            default:
                break;
            }
        }
        break;
    case Collider::BoundingBox::OBB_3D:
        {

            OBB colA = {
                    .center = wPosB,
                    .size = _colliderB->GetSize(),
                    .rotate = _colliderB->GetRotate()
            };
            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:
                {
                    Sphere colB = {
                   .center = wPosB,
                   .radius = _colliderB->GetRadius()
                    };

                    if (IsCollision(colA, colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            case Collider::BoundingBox::AABB_3D:
                {
                    AABB colB = {
                        .center = wPosB,
                        .min = -_colliderA->GetSize() / 2.0f,
                        .max = _colliderA->GetSize() / 2.0f,
                    };

                    if (IsCollision(colA, colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            case Collider::BoundingBox::OBB_3D:
                {
                    OBB colB = {
                        .center = wPosB,
                        .size = _colliderB->GetSize(),
                        .rotate = _colliderB->GetRotate()
                    };
                    colB.CalculateOrientations();
                    if (IsCollision(colA, colB))
                    {
                        _colliderA->OnCollision();
                        _colliderB->OnCollision();
                    }
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

}

bool CollisionManager::IsCollision(const Sphere& _s1, const Sphere& _s2)
{
    float distance = (_s1.center - _s2.center).Length();
    return distance <= _s1.radius + _s2.radius;
}

bool CollisionManager::IsCollision(const AABB& _a, const Sphere& _s)
{
    Vector3 closestPoint;
    closestPoint.x = std::clamp(_s.center.x, _a.min.x, _a.max.x);
    closestPoint.y = std::clamp(_s.center.y, _a.min.y, _a.max.y);
    closestPoint.z = std::clamp(_s.center.z, _a.min.z, _a.max.z);

    float distance = (closestPoint - _s.center).Length();

    if (distance <= _s.radius)
    {
        return true;
    }

    return false;
}

bool CollisionManager::IsCollision(const Sphere& _s, const AABB& _a)
{
    return IsCollision(_a, _s);
}

bool CollisionManager::IsCollision(const OBB& _obb, const Sphere& _sphere)
{
    Matrix4x4 obbWolrdMat = MakeAffineMatrix({ 1.0f,1.0f ,1.0f }, _obb.rotate, _obb.center);
    Matrix4x4 obbWorldMatInv = Inverse(obbWolrdMat);

    Vector3  centerInOBBLocalSphere = Transform(_sphere.center, obbWorldMatInv);
    AABB aabbOBBLocal{ .min = -_obb.size,.max = _obb.size };
    Sphere sphereOBBLocal{ centerInOBBLocalSphere,_sphere.radius };

    return IsCollision(aabbOBBLocal, sphereOBBLocal);
}
bool CollisionManager::IsCollision(const Sphere& _sphere, const OBB& _obb)
{
    return IsCollision(_obb, _sphere);
}
bool CollisionManager::IsCollision(const AABB& _aabb, const OBB& _obb)
{
    OBB obb = ConvertToAABBToOBB(_aabb);
    return IsCollision(obb, _obb);
}
bool CollisionManager::IsCollision(const OBB& _obb, const AABB& _aabb)
{
    return IsCollision(_aabb, _obb);;
}
bool CollisionManager::IsCollision(const OBB& _obb1, const OBB& _obb2)
{
    /// 分離軸候補の計算
    Vector3 axes[15];
    axes[0] = _obb1.orientations[0];
    axes[1] = _obb1.orientations[1];
    axes[2] = _obb1.orientations[2];

    axes[3] = _obb2.orientations[0];
    axes[4] = _obb2.orientations[1];
    axes[5] = _obb2.orientations[2];

    int index = 6;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            axes[index++] = _obb1.orientations[i].Cross(_obb2.orientations[j]);
        }
    }

    for (auto axis : axes)
    {
        float minObb1, maxObb1;
        float minObb2, maxObb2;

        //軸に射影および点の最大と最小を求める
        CalculateProjectionRange(_obb1, axis, minObb1, maxObb1);
        CalculateProjectionRange(_obb2, axis, minObb2, maxObb2);

        float l1, l2;
        l1 = maxObb1 - minObb1;
        l2 = maxObb2 - minObb2;

        float sumSpan = l1 + l2;

        float longSpan = std::max(maxObb1, maxObb2) - std::min(minObb1, minObb2);
        if (sumSpan < longSpan)
        {	//分離している	//すなわち衝突していない
            return false;
        }
    }
    return true;
}
void CollisionManager::CalculateProjectionRange(const OBB& _obb, const Vector3& _axis, float& _min, float& _max)
{
    Vector3 verties[8];
    _obb.CaluculateVertices(verties);

    _min = (float)0xffffffff;
    _max = -(float)0xffffffff;

    for (auto vertex : verties)
    {
        float proj = _axis.Dot(vertex);
        _min = std::min(_min, proj);
        _max = std::max(_max, proj);
    }

    return;
}

CollisionManager::OBB CollisionManager::ConvertToAABBToOBB(const AABB& _aabb)
{
    OBB obb;

    obb.center = _aabb.center;
    obb.size = _aabb.max * 2.0f;

    obb.orientations[0] = { 1.0f, 0.0f, 0.0f };
    obb.orientations[1] = { 0.0f, 1.0f, 0.0f };
    obb.orientations[2] = { 0.0f, 0.0f, 1.0f };

    obb.rotate = { 0.0f, 0.0f, 0.0f };
    return obb;
}

void CollisionManager::OBB::CalculateOrientations()
{
    Matrix4x4 rotateMatrix = MakeRotateMatrix(this->rotate);


    this->orientations[0].x = rotateMatrix.m[0][0];
    this->orientations[0].y = rotateMatrix.m[0][1];
    this->orientations[0].z = rotateMatrix.m[0][2];

    this->orientations[1].x = rotateMatrix.m[1][0];
    this->orientations[1].y = rotateMatrix.m[1][1];
    this->orientations[1].z = rotateMatrix.m[1][2];

    this->orientations[2].x = rotateMatrix.m[2][0];
    this->orientations[2].y = rotateMatrix.m[2][1];
    this->orientations[2].z = rotateMatrix.m[2][2];

    this->orientations[0] = this->orientations[0].Normalize();
    this->orientations[1] = this->orientations[1].Normalize();
    this->orientations[2] = this->orientations[2].Normalize();
}

void CollisionManager::OBB::CaluculateVertices(Vector3* vertices) const
{
    Vector3 rotateAxis[3];
    rotateAxis[0] = this->orientations[0] * this->size.x;
    rotateAxis[1] = this->orientations[1] * this->size.y;
    rotateAxis[2] = this->orientations[2] * this->size.z;

    vertices[0] = this->center + rotateAxis[0] + rotateAxis[1] + rotateAxis[2];
    vertices[1] = this->center + rotateAxis[0] + rotateAxis[1] - rotateAxis[2];
    vertices[2] = this->center + rotateAxis[0] - rotateAxis[1] + rotateAxis[2];
    vertices[3] = this->center + rotateAxis[0] - rotateAxis[1] - rotateAxis[2];
    vertices[4] = this->center - rotateAxis[0] + rotateAxis[1] + rotateAxis[2];
    vertices[5] = this->center - rotateAxis[0] + rotateAxis[1] - rotateAxis[2];
    vertices[6] = this->center - rotateAxis[0] - rotateAxis[1] + rotateAxis[2];
    vertices[7] = this->center - rotateAxis[0] - rotateAxis[1] - rotateAxis[2];
}
