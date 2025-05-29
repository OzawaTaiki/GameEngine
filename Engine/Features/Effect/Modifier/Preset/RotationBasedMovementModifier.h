#pragma once

#include <Features/Effect/Modifier/ParticleModifier.h>

class RotationBasedMovementModifier : public ParticleModifier
{
public:

    RotationBasedMovementModifier() = default;
    ~RotationBasedMovementModifier() override = default;

    void Apply(Particle* particle, float deltaTime) override;

};