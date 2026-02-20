#pragma once

#include <Features/Effect/Modifier/ParticleModifier.h>


namespace Engine {

class RotationBasedMovementModifier : public ParticleModifier
{
public:

    RotationBasedMovementModifier() = default;
    ~RotationBasedMovementModifier() override = default;

    void Apply(Particle* particle, float deltaTime) override;

};

} // namespace Engine
