#pragma once

#include <Features/Effect/Particle/Particle.h>


namespace Engine {

class ParticleModifier
{
public:

    virtual ~ParticleModifier() = default;

    virtual void Initialize() {};

    virtual void Apply(Particle* _particle, float _deltaTime) = 0;
    virtual void Apply(std::list<Particle*> _particles, float _deltaTime) {
        for (auto& particle : _particles)
        {
            Apply(particle, _deltaTime);
        }
    }

private:


};

} // namespace Engine
