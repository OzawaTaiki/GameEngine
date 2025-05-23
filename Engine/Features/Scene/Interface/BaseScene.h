#pragma once

class BaseScene
{
public:

    virtual ~BaseScene() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void DrawShadow() = 0;

protected:


};
