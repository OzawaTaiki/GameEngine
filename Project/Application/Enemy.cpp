#include "Enemy.h"
#include "collisionManager.h"

void Enemy::Initialize(const Vector3& _pos, const Vector3& _velo, float _lifeTime)
{
    model_ = Model::CreateFromObj("bunny.gltf");

    worldTransform_.Initialize();
    worldTransform_.transform_ = _pos;
    worldTransform_.UpdateData();

    velocity_ = _velo;
    lifeTime_ = _lifeTime;

    Collider::SetAtrribute("Enemy");
    Collider::SetMask({ "Enemy" });
    Collider::SetBoundingBox(Collider::BoundingBox::Sphere_3D);

    size_ = (model_->GetMin() - model_->GetMax())/ 2.0f;
    pRotate_ = &worldTransform_.rotate_;
    offset_ = (model_->GetMax() + model_->GetMin()) / 2.0f;
    pWorldTransform_ = &worldTransform_.matWorld_;


}

void Enemy::Update()
{
    static const float kDeleteTime = 1.0f / 60.0f;
    lifeTime_ -= kDeleteTime;
    if (lifeTime_ <= 0.0f)
    {
        isAlive_ = false;
    }

    worldTransform_.transform_ += velocity_ * kDeleteTime;

    if(isAlive_)
        CollisionManager::GetInstance()->SetCollider(this);

    worldTransform_.UpdateData();
}

void Enemy::Draw(const Camera* _camera)
{
    if(isAlive_)
        model_->Draw(worldTransform_, _camera);

    if (IsDrawBoundingBox_)
    {
        Collider::Draw(worldTransform_.matWorld_);
    }
}

void Enemy::OnCollision()
{
    isAlive_ = false;
}