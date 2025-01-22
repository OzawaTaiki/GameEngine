#pragma once

#include <ISceneFactory.h>

class SceneFactory : public ISceneFactory
{
public:
    std::unique_ptr<BaseScene> CreateScene(const std::string& _name) override;
};
