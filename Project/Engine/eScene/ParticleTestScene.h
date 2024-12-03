#pragma once
#include "BaseScene.h"

#include "Particle/ParticleManager.h"
#include "Particle/ParticleEmitters.h"
#include "Particle/Effect.h"

#include <list>
#include <memory>

class ParticleTestScene : public BaseScene
{
public:

    static std::unique_ptr<BaseScene> Create();

     ~ParticleTestScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

    std::list<Effect> effects_ = {};

    std::list<std::unique_ptr<ParticleEmitter>> emitters_ = {};


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};