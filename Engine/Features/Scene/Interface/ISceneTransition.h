#pragma once


namespace Engine {

class ISceneTransition
{
public:
    virtual ~ISceneTransition() = default;

    virtual void Initialize() = 0;

    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Start() = 0;
    virtual void End() = 0;

    virtual bool IsEnd() = 0;
    virtual bool CanSwitch() = 0;


};

} // namespace Engine
