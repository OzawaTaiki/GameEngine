#pragma once

#include <Features/Effect/Modifier/IPaticleMoifierFactory.h>

class ParticleModifierFactory : public Engine::IParticleMoifierFactory
{

public:
    ParticleModifierFactory() = default;
    ~ParticleModifierFactory() override = default;

    std::unique_ptr<Engine::ParticleModifier> CreateModifier(const std::string& _name) override;
};