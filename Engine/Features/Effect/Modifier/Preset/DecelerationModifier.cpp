#include "DecelerationModifier.h"

float DecelerationModifier::deceleration_ = 0.8f; // 初期値

void DecelerationModifier::Apply(Particle* _particle, float _deltaTime)
{
    if (_particle == nullptr)
        return;
    // 減速率を適用
    float speed = _particle->GetSpeed();
    speed *= (1.0f - deceleration_ * _deltaTime);
    _particle->SetSpeed(speed);
}
