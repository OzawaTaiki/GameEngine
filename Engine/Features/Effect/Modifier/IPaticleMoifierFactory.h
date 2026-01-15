#pragma once

#include <Features/Effect/Modifier/ParticleModifier.h>
#include <memory>
#include <string>


namespace Engine {

class IParticleMoifierFactory
{
public:
    virtual ~IParticleMoifierFactory() = default;

    virtual std::unique_ptr<ParticleModifier> CreateModifier(const std::string& _name) = 0;

};

} // namespace Engine
