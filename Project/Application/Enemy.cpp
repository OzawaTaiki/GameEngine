#include "Enemy.h"
#include "../Collider/CollisionManager.h"

void Enemy::Initialize(const Vector3& _pos, const Vector3& _velo, float _lifeTime)
{
    model_ = Model::CreateFromObj("bunny.gltf");

    worldTransform_.Initialize();
    worldTransform_.transform_ = _pos;
    worldTransform_.UpdateData();

    velocity_ = _velo;
    lifeTime_ = _lifeTime;

    collider_ = std::make_unique<Collider>();
    collider_->SetBoundingBox(Collider::BoundingBox::AABB_3D);
    collider_->SetShape(model_->GetMin(), model_->GetMax());
    collider_->SetAtrribute("Enemy");
    collider_->SetMask("Enemy");
    collider_->SetGetWorldMatrixFunc([this]() {return worldTransform_.matWorld_; });
    collider_->SetOnCollisionFunc([this]() {OnCollision(); });
    collider_->SetReferencePoint({ 0,(model_->GetMax().y) / 2.0f,0 });


    isAlive_ = true;
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

    if (isAlive_)
    {
        CollisionManager::GetInstance()->RegisterCollider(collider_.get());
    }
    worldTransform_.UpdateData();
}

void Enemy::Draw(const Camera* _camera)
{
    if(isAlive_)
        model_->Draw(worldTransform_, _camera);

#ifdef _DEBUG
    if (IsDrawBoundingBox_)
    {
        collider_->Draw();
    }
#endif // _DEBUG
}

void Enemy::OnCollision()
{
    isAlive_ = false;
}
