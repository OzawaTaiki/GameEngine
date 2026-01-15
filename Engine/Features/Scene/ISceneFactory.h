#pragma once

#include <Features/Scene/Interface/BaseScene.h>

#include <memory>
#include <string>

/// <summary>
/// シーンファクトリ インタフェース
/// </summary>

namespace Engine {

class ISceneFactory
{
public:
    virtual ~ISceneFactory() = default;

    virtual std::unique_ptr<BaseScene> CreateScene(const std::string& _name) = 0;

    virtual std::string ShowDebugWindow() = 0;
};

} // namespace Engine
