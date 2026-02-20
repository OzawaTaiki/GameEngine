#pragma once

#include <Features/Scene/SceneData.h>


namespace Engine {

class BaseScene
{
public:

    virtual ~BaseScene() = default;

    virtual void Initialize(SceneData* _sceneData) = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void DrawShadow() = 0;

protected:


};

} // namespace Engine
