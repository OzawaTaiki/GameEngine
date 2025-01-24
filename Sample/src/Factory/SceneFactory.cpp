#include "SceneFactory.h"

#include <src/Scene/SampleScene.h>
#include <src/Scene/FeatureScene.h>

#include <Features/Scene/ParticleTestScene.h>

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
    else if (_name == "Feature")
    {
        return std::make_unique<FeatureScene>();
    }

    assert("Scene Not Found");

    return nullptr;
}
