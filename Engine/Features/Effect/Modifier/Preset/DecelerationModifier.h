#pragma once

#include <Features/Effect/Modifier/ParticleModifier.h>

class DecelerationModifier : public ParticleModifier
{
public:
    DecelerationModifier() = default;
    ~DecelerationModifier() override = default;

    void Apply(Particle* _particle, float _deltaTime) override;

    static void SetDeceleration(float _deceleration) { deceleration_ = _deceleration; }

private:

    static float deceleration_; // 減速率

};