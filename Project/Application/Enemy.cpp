#include "Enemy.h"
#include "collisionManager.h"

void Enemy::Initialize(const Vector3& _pos)
{
    model_ = Model::CreateFromObj("bunny.gltf");

    worldTransform_.Initialize();
    worldTransform_.transform_=_pos;
    worldTransform_.UpdateData();

    Collider::SetAtrribute("Enemy");
    Collider::SetMask({ "Enemy" });
    Collider::SetBoundingBox(Collider::BoundingBox::Sphere_3D);


    size_ = (model_->GetMin() - model_->GetMax())/ 2.0f;
    pRotate_ = &worldTransform_.rotate_;
    offset_ = (model_->GetMax() + model_->GetMin()) / 2.0f;

}

void Enemy::Update()
{
    //if(isAlive_)
        CollisionManager::GetInstance()->SetCollider(this);

    worldTransform_.UpdateData();
}

void Enemy::Draw(const Camera* _camera)
{
    if(isAlive_)
        model_->Draw(worldTransform_, _camera);

    if (IsDrawBoundingBox_)
    {
        Collider::Draw();
    }
}

void Enemy::OnCollision()
{
    isAlive_ = false;
}
