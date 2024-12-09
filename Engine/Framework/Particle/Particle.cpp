#include "Particle.h"
#include <Physics/Math/MatrixFunction.h>
#include <Physics/Math/MyLib.h>

#include <algorithm>

void Particle::Initialize(const ParticleInitParam& _param)
{
    parameter_ = _param;

    lifeTime_ = parameter_.lifeTime;

    translate_ = parameter_.position;
    rotation_ = parameter_.rotate;
    scale_ = parameter_.size;
    speed_ = parameter_.speed;

    direction_ = parameter_.direction;
    acceleration_ = parameter_.acceleration;

    deceleration_ = parameter_.deceleration;

    color_ = parameter_.color;



    isAlive_ = true;
    currentTime_ = 0.0f;

    velocity_ = direction_.Normalize() * speed_;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

    directionMatrix_ = _param.directionMatrix;

    parameter_.alphaTransition .keys.sort    ([](const KeyFrame<float  >& a, const KeyFrame< float >& b)     {return a.time < b.time; });
    parameter_.colorTransition .keys.sort    ([](const KeyFrame<Vector3>& a, const KeyFrame<Vector3>& b)     {return a.time < b.time; });
    parameter_.rotateTransition.keys.sort   ([](const KeyFrame<Vector3>& a, const KeyFrame<Vector3>& b)     {return a.time < b.time; });
    parameter_.sizeTransition  .keys.sort     ([](const KeyFrame<Vector3>& a, const KeyFrame<Vector3>& b)     {return a.time < b.time; });
    parameter_.speedTransition .keys.sort    ([](const KeyFrame<float  >& a, const KeyFrame< float >& b)     {return a.time < b.time; });

    t_ = 0;
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

    t_ = currentTime_ / lifeTime_;

    color_ = parameter_.colorTransition.calculateValue(t_);
    color_.w = parameter_.alphaTransition.calculateValue(t_);
    rotation_ = parameter_.rotateTransition.calculateValue(t_);
    scale_ = parameter_.sizeTransition.calculateValue(t_);
    speed_ = parameter_.speedTransition.calculateValue(t_);


    velocity_ = direction_.Normalize() * speed_;

    velocity_ += acceleration_ * currentTime_;

    if (deceleration_ != 0)
    velocity_ -= velocity_ * deceleration_ * kDeltaTime;


    translate_ += velocity_ * kDeltaTime;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

}

void Particle::Draw()
{
}
