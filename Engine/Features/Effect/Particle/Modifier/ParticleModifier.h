#pragma once

#include <Features/Effect/Particle/Particle.h>

class ParticleModifier
{
public:

    virtual ~ParticleModifier() = default;

    virtual void Initialize() {};

    virtual void Apply(Particle* _particle, float _deltaTime) = 0;

private:


};