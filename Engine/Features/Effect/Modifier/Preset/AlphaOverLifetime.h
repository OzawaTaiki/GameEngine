#pragma once

#include <Features/Effect/Modifier/ParticleModifier.h>
#include <Features/Effect/Particle/Particle.h>

#include <Math/Easing.h>

class AlphaOverLifetime : public ParticleModifier
{
public:
    AlphaOverLifetime() = default;
    ~AlphaOverLifetime() = default;

    void Apply(Particle* _particle, float _deltaTime) override;

    static void SetEasingType(Easing::EasingFunc _easingType) { easingType_ = _easingType; }

private:

    static Easing::EasingFunc easingType_; // イージングタイプ

};
