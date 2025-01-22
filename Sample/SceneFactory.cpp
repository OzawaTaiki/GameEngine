#include "SceneFactory.h"

#include "SampleScene.h"
#include <Framework/eScene/ParticleTestScene.h>

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& _name)
{
    if (_name == "Sample")
    {
        return std::make_unique<SampleScene>();
    }
    else if (_name == "ParticleTest")
    {
        return std::make_unique<ParticleTestScene>();
    }

    assert("Scene Not Found");

    return nullptr;
}
