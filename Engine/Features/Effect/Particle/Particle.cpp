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

    if(parameter_.sequence)
        parameter_.sequence->SetCurrentTime(0);

    // ビルボードの初期化
    isBillboard_[0] = (parameter_.billboard & 0x1) != 0;
    isBillboard_[1] = (parameter_.billboard & 0x2) != 0;
    isBillboard_[2] = (parameter_.billboard & 0x4) != 0;

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

    if (parameter_.sequence != nullptr)
    {
        auto sequence = parameter_.sequence;

        sequence->Update(_deltaTime);

        if (sequence->HasEvent("color_RGB"))
            color_ = sequence->GetValue<Vector3>("color_RGB");
        if (sequence->HasEvent("color_A"))
            color_.w = sequence->GetValue<float>("color_A");

        if (sequence->HasEvent("rotate"))
            rotation_ = sequence->GetValue<Vector3>("rotate");
        if (sequence->HasEvent("size"))
            scale_ = sequence->GetValue<Vector3>("size");
        if (sequence->HasEvent("speed"))
            speed_ = sequence->GetValue<float>("speed");
        if (sequence->HasEvent("direction"))
            direction_ = sequence->GetValue<Vector3>("direction");
        if (sequence->HasEvent("acceleration"))
            acceleration_ = sequence->GetValue<Vector3>("acceleration");
        if (sequence->HasEvent("position"))
            translate_ = sequence->GetValue<Vector3>("position");

        // TODO: シーケンスでUVを変更してもいいかも


    }


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
