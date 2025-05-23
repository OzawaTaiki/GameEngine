#include "ParticleModifierFactory.h"

#include <Features/Effect/Modifier/Preset/DecelerationModifier.h>
#include <Features/Effect/Modifier/Preset/AlphaOverLifetime.h>

std::unique_ptr<ParticleModifier> ParticleModifierFactory::CreateModifier(const std::string _name)
{
    if (_name == "DecelerationModifier")
    {
        return std::make_unique<DecelerationModifier>();
    }
    else if (_name == "AlphaOverLifetime")
    {
        return std::make_unique<AlphaOverLifetime>();
    }
    else if (_name == "ParticleModifier2")
    {
        // return std::make_unique<ParticleModifier2>();
    }


    throw std::runtime_error("モディファイアが見つかりませんでした。");

    return nullptr;
}
