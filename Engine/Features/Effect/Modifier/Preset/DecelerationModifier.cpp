#include "DecelerationModifier.h"

#include <algorithm>


namespace Engine {

float DecelerationModifier::deceleration_ = 2.0f; // 初期値

void DecelerationModifier::Apply(Particle* _particle, float _deltaTime)
{
    if (_particle == nullptr)
        return;
    // 減速率を適用
    float speed = _particle->GetSpeed();
    speed *= (std::max)(0.0f, 1.0f - deceleration_ * _deltaTime);
    _particle->SetSpeed(speed);
}

} // namespace Engine
