#define NOMINMAX

#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>

void Collider::Update()
{
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Sphere>)
        {
            Sphere& sphere = arg;
            sphere.center = Transform(sphere.referencePoint, GetWorldMatrix());

        }
        else if constexpr (std::is_same_v<T, AABB>)
        {
            AABB& aabb = arg;
            aabb.Calculate(GetWorldMatrix());
        }
        else if constexpr (std::is_same_v<T, OBB>)
        {
            OBB& obb = arg;
            obb.Calculate(GetWorldMatrix());
        }
        else
        {
            assert(false && "Not supported");
        }
               }, shape_);


}

void Collider::Draw()
{
    Matrix4x4 worldMat = GetWorldMatrix();

    auto lineDrawer = LineDrawer::GetInstance();

    std::visit([&worldMat, lineDrawer](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Sphere>)
        {
            Sphere& sphere = arg;
            worldMat = MakeScaleMatrix(sphere.radius) * worldMat;
            worldMat = MakeTranslateMatrix(sphere.referencePoint) * worldMat;
            lineDrawer->DrawSphere(worldMat);
        }
        else if constexpr (std::is_same_v<T, AABB>)
        {
            AABB& aabb = arg;
            aabb.Calculate(worldMat);
            lineDrawer->DrawOBB(aabb.vertices);
        }
        else if constexpr (std::is_same_v<T, OBB>)
        {
            OBB& obb = arg;
            obb.Calculate(worldMat);
            lineDrawer->DrawOBB(obb.vertices);
        }
        else
        {
            assert(false && "Not supported");
        }
               }, shape_);


}

void Collider::SetShape(float _radius)
{
    switch (boundingBox_)
    {
    case Collider::BoundingBox::Sphere_3D:
        shape_ = Sphere{ _radius };
        break;
    case Collider::BoundingBox::AABB_3D:
    case Collider::BoundingBox::OBB_3D:
        assert(false && "SetShape(float _radius) is not supported in AABB_3D and OBB_3D");
        break;
    case Collider::BoundingBox::NONE:
        assert(false && "Please set BoundingBox before SetShape");
        break;
    default:
        break;
    }

    preSize_ = _radius * 1.1f;
}

void Collider::SetShape(const Vector3& _min, const Vector3& _max)
{
    switch (boundingBox_)
    {
    case Collider::BoundingBox::Sphere_3D:
        assert(false && "SetShape(const Vector3& _min, const Vector3& _max) is not supported in Sphere_3D");
        break;
    case Collider::BoundingBox::AABB_3D:
        shape_ = AABB(_min, _max);
        break;
    case Collider::BoundingBox::OBB_3D:
        shape_ = OBB(_min, _max);
        break;
    case Collider::BoundingBox::NONE:
        assert(false && "Please set BoundingBox before SetShape");
        break;
    default:
        break;
    }

    Vector3 size = (_max - _min) * 1.1f;

    float min = std::min(size.x, std::min(size.y, size.z));
    float max = std::max(size.x, std::max(size.y, size.z));
    // 少し大きめに半径を設定する
    preSize_ = max - min;
}


void Collider::OnCollision(const Collider* _other)
{
    if (fOnCollision_)
    {
        isHit_ = true;
        fOnCollision_(_other);
    }
}

void Collider::SetAtrribute(const std::string& _atrribute)
{
    atrribute_ = CollisionManager::GetInstance()->GetAtttibute(_atrribute);
    if (name_.empty())
        name_ = _atrribute;
}

void Collider::SetMask(const std::string& _atrribute)
{
    mask_ = CollisionManager::GetInstance()->GetMask(_atrribute);
}

void Collider::SetMask(std::initializer_list<std::string> _atrribute)
{
    mask_ = 0xffffffffu;
    for (std::string str : _atrribute)
    {
        mask_ &= CollisionManager::GetInstance()->GetMask(str);
    }
}

void Collider::SetReferencePoint(const Vector3& _referencePoint)
{
    if (boundingBox_ != BoundingBox::NONE)
    {
        std::visit([_referencePoint](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, OBB>)
            {
                OBB& obb = arg;
                obb.referencePoint[0] = _referencePoint.x;
                obb.referencePoint[1] = _referencePoint.y;
                obb.referencePoint[2] = _referencePoint.z;
            }
            else if constexpr (std::is_same_v<T, AABB>)
            {
                AABB& aabb = arg;
                aabb.referencePoint[0] = _referencePoint.x;
                aabb.referencePoint[1] = _referencePoint.y;
                aabb.referencePoint[2] = _referencePoint.z;
            }
            else if constexpr (std::is_same_v<T, Sphere>)
            {
                Sphere& sphere = arg;
                sphere.referencePoint[0] = _referencePoint.x;
                sphere.referencePoint[1] = _referencePoint.y;
                sphere.referencePoint[2] = _referencePoint.z;
            }
            else
            {
                assert(false && "Not supported");

            }
                   }, shape_);
    }
    else
    {
        assert(false && "Not supported");
    }
}

void Collider::RegsterCollider()
{
    Update();
    CollisionManager::GetInstance()->RegisterCollider(this);
}
