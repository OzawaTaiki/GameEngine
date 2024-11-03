#include "Particle.h"
#include "MatrixFunction.h"

void Particle::Initialize(float _lifeTime,
                          const Vector3& _size,
                          const Vector3& _rotate,
                          const Vector3& _pos,
                          const Vector4& _color,
                          float _speed,
                          const Vector3& _direction,
                          const Vector3& _acceleration)
{
    lifeTime_ = _lifeTime;
    scale_ = _size;
    rotation_ = _rotate;
    translate_ = _pos;
    color_ = _color;
    speed_ = _speed;
    direction_ = _direction;
    acceleration_ = _acceleration;

    isAlive_ = true;
    currentTime_ = 0;

    velocity_ = direction_.Normalize() * speed_;
    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);
}

void Particle::Update()
{
    const float kDeltaTime = 1.0f / 60.0f;
    currentTime_ += kDeltaTime;

    if (currentTime_ >= lifeTime_)
    {
        isAlive_ = false;
        return;
    }

    velocity_ += acceleration_ * kDeltaTime;
    translate_ += velocity_ * kDeltaTime;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

}

void Particle::Draw()
{
}