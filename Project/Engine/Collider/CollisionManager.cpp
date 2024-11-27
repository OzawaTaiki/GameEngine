#include "CollisionManager.h"
#include "Collider.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include <algorithm>

CollisionManager* CollisionManager::GetInstance()
{
    static CollisionManager instance;
    return &instance;
}

uint32_t CollisionManager::GetAtttibute(const std::string& _atrribute)
{
    auto it = atrributeMap_.find(_atrribute);
    if (it != atrributeMap_.end())
        return it->second;

    atrributeMap_[_atrribute] = static_cast <uint32_t>(1 << atrributeMap_.size());
    uint32_t result = atrributeMap_[_atrribute];
    return result;
}

uint32_t CollisionManager::GetMask(const std::string& _atrribute)
{
    return ~GetAtttibute(_atrribute);
}

void CollisionManager::RegisterCollider(Collider* _collider)
{
    _collider->Update();
    colliders_.push_back(_collider);
}

void CollisionManager::CheckAllCollision()
{
    for (auto itA = colliders_.begin(); itA != colliders_.end(); itA++)
    {
        (*itA)->NotHit();
        for (auto itB = std::next(itA); itB != colliders_.end(); itB++)
        {
            (*itB)->NotHit();
            if ((*itA)->GetMask() & (*itB)->GetAtrribute_() &&
                (*itB)->GetMask() & (*itA)->GetAtrribute_())
            {
                CheckCollisionPair(*itA, *itB);
            }
        }
    }
}

void CollisionManager::CheckCollisionPair(Collider* _colliderA, Collider* _colliderB)
{
    switch (_colliderA->GetBoundingBox())
    {
    case Collider::BoundingBox::Sphere_3D:
        {
            Sphere sphereA = _colliderA->GetShape<Sphere>();
            sphereA.center = Transform(sphereA.referencePoint, _colliderA->GetWorldMatrix());

            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:
                {
                    Sphere sphereB = _colliderB->GetShape<Sphere>();
                    sphereB.center = Transform(sphereA.referencePoint, _colliderB->GetWorldMatrix());

                    if (IsCollision(sphereA, sphereB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                }
                break;
            case Collider::BoundingBox::AABB_3D:
                {
                    AABB aabbB = _colliderB->GetShape<AABB>();
                    aabbB.center = Transform({ 0,0,0 }, _colliderB->GetWorldMatrix());

                    if (IsCollision(sphereA, aabbB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::OBB_3D:
                {
                    OBB obbB = _colliderB->GetShape<OBB>();
                    //obbB.center = Transform(obbB.referencePoint, _colliderB->GetWorldMatrix());
                    if (IsCollision(sphereA, obbB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::NONE:
                assert(false && "Please set BoundingBox before SetShape");
                break;
            default:
                break;
                }
            }
        break;
    case Collider::BoundingBox::AABB_3D:
        {
            AABB aabbA = _colliderA->GetShape<AABB>();
            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:
                {
                    Sphere sphereB = _colliderB->GetShape<Sphere>();
                    if (IsCollision(sphereB, aabbA))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::AABB_3D:
                {
                    AABB aabbB = _colliderB->GetShape<AABB>();
                    if (IsCollision(aabbA, aabbB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::OBB_3D:
                {
                    OBB obbB = _colliderB->GetShape<OBB>();
                    if (IsCollision(aabbA, obbB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::NONE:
                assert(false && "Please set BoundingBox before SetShape");
                break;
            default:
                break;
            }
        }
        break;
    case Collider::BoundingBox::OBB_3D:
            {
                OBB obbA = _colliderA->GetShape<OBB>();
            switch (_colliderB->GetBoundingBox())
            {
            case Collider::BoundingBox::Sphere_3D:
                {
                    Sphere sphereB = _colliderB->GetShape<Sphere>();
                    if (IsCollision(sphereB, obbA))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::AABB_3D:
                {
                    AABB aabbB = _colliderB->GetShape<AABB>();
                    if (IsCollision(aabbB, obbA))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;
                }
            case Collider::BoundingBox::OBB_3D:
                {
                    OBB obbB = _colliderB->GetShape<OBB>();
                    if (IsCollision(obbA, obbB))
                    {
                        _colliderA->OnCollision(_colliderB);
                        _colliderB->OnCollision(_colliderA);
                    }
                    break;


                }
                break;
            }
        }
        break;
    case Collider::BoundingBox::NONE:
        assert(false && "Please set BoundingBox before SetShape");
        break;
    default:
        break;
    }
};

bool CollisionManager::IsCollision(const Sphere& _sphere1, const Sphere& _sphere2)
{
    float distance = Length(Subtract(_sphere1.center, _sphere2.center));
    return distance <= _sphere1.radius + _sphere2.radius;
}

bool CollisionManager::IsCollision(const Sphere& _sphere, const AABB& _aabb)
{
    Vector3 closestPoint;
    closestPoint.x = std::clamp(_sphere.center.x, _aabb.min.x, _aabb.max.x);
    closestPoint.y = std::clamp(_sphere.center.y, _aabb.min.y, _aabb.max.y);
    closestPoint.z = std::clamp(_sphere.center.z, _aabb.min.z, _aabb.max.z);

    float distance = Length(closestPoint - _sphere.center);

    if (distance <= _sphere.radius)
    {
        return true;
    }

    return false;
}

bool CollisionManager::IsCollision(const Sphere& _sphere, const OBB& _obb)
{
    Matrix4x4 obbWorldMatInv = Inverse(_obb.worldMat);

    Vector3  centerInOBBLocalSphere = Transform(_sphere.center, obbWorldMatInv);
    AABB aabbOBBLocal;// = { _obb.min,_obb.max };
    aabbOBBLocal.min = Transform(_obb.min, obbWorldMatInv);
    aabbOBBLocal.max = Transform(_obb.max, obbWorldMatInv);
    for (int i = 0; i < 3; i++)
    {
        if (aabbOBBLocal.min[i] > aabbOBBLocal.max[i])
        {
            std::swap(aabbOBBLocal.min[i], aabbOBBLocal.max[i]);
        }
    }

    Sphere sphereOBBLocal;
    sphereOBBLocal.center = centerInOBBLocalSphere;
    sphereOBBLocal.radius = _sphere.radius;

    return IsCollision(sphereOBBLocal, aabbOBBLocal);
}


bool CollisionManager::IsCollision(const AABB& _aabb1, const AABB& _aabb2)
{
    if ((_aabb1.min.x <= _aabb2.max.x && _aabb1.max.x >= _aabb2.min.x) && // x
        (_aabb1.min.y <= _aabb2.max.y && _aabb1.max.y >= _aabb2.min.y) && // y
        (_aabb1.min.z <= _aabb2.max.z && _aabb1.max.z >= _aabb2.min.z)) { // z
        //衝突
        return true;
    }

    return false;
}

bool CollisionManager::IsCollision(const AABB& _aabb, const OBB& _obb)
{
    // AABBをOBBに変換
    OBB aabbConverted;
    aabbConverted = AABBConvertToOBB(_aabb);

    return IsCollision(aabbConverted, _obb);
}

bool CollisionManager::IsCollision(const OBB& _obb1, const OBB& _obb2)
{
    //分離軸候補の計算
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
			axes[index++] = Cross(_obb1.orientations[i], _obb2.orientations[j]);
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
	_min = std::numeric_limits<float>::infinity();
	_max = -(float)std::numeric_limits<float>::infinity();

	for (auto vertex : _obb.vertices)
	{
		float proj = Dot(_axis, vertex);
		_min = std::min(_min, proj);
		_max = std::max(_max, proj);
	}

	return;
}

OBB CollisionManager::AABBConvertToOBB(const AABB& _aabb)
{
    OBB obb;
    obb.min = _aabb.min;
    obb.max = _aabb.max;
    obb.orientations[0] = { 1,0,0 };
    obb.orientations[1] = { 0,1,0 };
    obb.orientations[2] = { 0,0,1 };
    obb.vertices = _aabb.vertices;

    return obb;
}
