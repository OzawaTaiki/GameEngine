#include <Features/Effect/Particle/Particle.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/MyLib.h>

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

    color_ = parameter_.color;

    isAlive_ = true;
    isInfiniteLife_ = parameter_.isInfiniteLife;
    currentTime_ = 0.0f;

    velocity_ = direction_.Normalize() * speed_;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

    directionMatrix_ = _param.directionMatrix;

    t_ = 0;
}

void Particle::Update(float _deltaTime)
{
    currentTime_ += _deltaTime;

    if (currentTime_ >= lifeTime_ && !isInfiniteLife_)
    {
        isAlive_ = false;
        return;
    }

    t_ = currentTime_ / lifeTime_;

    if (!parameter_.colorTransition.keys.empty())
        color_ = parameter_.colorTransition.calculateValue(t_);
    if (!parameter_.alphaTransition.keys.empty())
        color_.w = parameter_.alphaTransition.calculateValue(t_);
    if (!parameter_.rotateTransition.keys.empty())
        rotation_ = parameter_.rotateTransition.calculateValue(t_);
    if (!parameter_.sizeTransition.keys.empty())
        scale_ = parameter_.sizeTransition.calculateValue(t_);
    if (!parameter_.speedTransition.keys.empty())
        speed_ = parameter_.speedTransition.calculateValue(t_);


    velocity_ = direction_.Normalize() * speed_;

    velocity_ += acceleration_ * currentTime_;

    if (deceleration_ != 0)
        velocity_ -= velocity_ * deceleration_ * _deltaTime;


    translate_ += velocity_ * _deltaTime;

    matWorld_ = MakeAffineMatrix(scale_, rotation_, translate_);

}

void Particle::Draw()
{
}

void Particle::ShowDebugWindow()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    ImGui::DragFloat("LifeTime", &lifeTime_);
    ImGui::DragFloat3("Position", &translate_.x);
    ImGui::DragFloat3("Rotation", &rotation_.x);
    ImGui::DragFloat3("Scale", &scale_.x);
    ImGui::DragFloat("Speed", &speed_);
    ImGui::DragFloat3("Direction", &direction_.x);
    ImGui::DragFloat3("Acceleration", &acceleration_.x);
    ImGui::ColorEdit4("Color", &color_.x);
    ImGui::Checkbox("InfiniteLife", &isInfiniteLife_);
    ImGui::PopID();
#endif // _DEBUG
}
