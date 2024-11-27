#include "Particle.h"
#include "MatrixFunction.h"
#include "MyLib.h"

void Particle::Initialize(float _lifeTime,
                          const Vector3& _size,
                          const Vector3& _rotate,
                          const Vector3& _pos,
                          const Vector4& _color,
                          float _speed,
                          const Vector3& _direction,
                          const Vector3& _acceleration,
                          bool _fade,
                          float _faderatio)
{
    lifeTime_ = _lifeTime;
    scale_ = _size;
    rotation_ = _rotate;
    translate_ = _pos;
    color_ = _color;
    speed_ = _speed;
    direction_ = _direction;
    acceleration_ = _acceleration;
    if (isFade_ = _fade; isFade_)
        fadeRatio_ = _faderatio;
    else fadeRatio_ = 1.0f;

    isAlive_ = true;
    currentTime_ = 0;

    velocity_ = direction_.Normalize() * speed_;
    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);
}

void Particle::Initialize(const ParticleInitParam& _param)
{
    lifeTime_ = _param.lifeTime;
    scale_ = _param.currentSize;
    rotation_ = _param.rotate;
    translate_ = _param.position;
    color_ = _param.currentColor;
    speed_ = _param.speed;
    direction_ = _param.direction;
    acceleration_ = _param.acceleration;

    if (isChangeColor_ = _param.changeColor; isChangeColor_)
    {
        startColor_ = _param.startColor;
        endColor_ = _param.endColor;
    }

    if (isChangeScale_ = _param.changeSize; isChangeScale_)
    {
        minScale_ = _param.startSize;
        maxScale_ = _param.endSize;
    }

    if (isFade_ = _param.isFade; isFade_)
        fadeRatio_ = _param.fadeRatio;

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

    if (isChangeColor_)
        color_ = Lerp(startColor_, endColor_, currentTime_ / lifeTime_);

    if(isChangeScale_)
        scale_ = Lerp(minScale_, maxScale_, currentTime_ / lifeTime_);

    if (isFade_)
    {
        float t = currentTime_ / lifeTime_;
        if (t >= fadeRatio_)
        {
            color_.w = 1.0f - (t - fadeRatio_) / (1.0f - fadeRatio_);
        }
    }

    velocity_ += acceleration_ * kDeltaTime;
    translate_ += velocity_ * kDeltaTime;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

}

void Particle::Draw()
{
}