#pragma once

#include <Features/Scene/ISceneFactory.h>

class SceneFactory : public Engine::ISceneFactory
{
public:
    std::unique_ptr<Engine::BaseScene> CreateScene(const std::string& _name) override;

    std::string ShowDebugWindow() override;

};
