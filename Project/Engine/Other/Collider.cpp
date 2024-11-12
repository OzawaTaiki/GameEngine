#include "Collider.h"
#include "CollisionManager.h"
#include "LineDrawer.h"
#include "MatrixFunction.h"

void Collider::Draw(Matrix4x4 _world) const
{
    Matrix4x4 affine = MakeTranslateMatrix(offset_) * _world;
    switch (boundingBox_)
    {
    case Collider::BoundingBox::Sphere_3D:
        LineDrawer::GetInstance()->DrawSphere(affine);
        break;
    case Collider::BoundingBox::AABB_3D:
    case Collider::BoundingBox::OBB_3D:
        LineDrawer::GetInstance()->DrawOBB(affine);
        break;
    default:
        break;
    }
}

void Collider::SetAtrribute(const std::string& _atrribute)
{
    atrribute_ = CollisionManager::GetInstance()->GetAtttibute(_atrribute);
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
