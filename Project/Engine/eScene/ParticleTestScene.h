#pragma once
#include "BaseScene.h"

#include "Particle/ParticleManager.h"
#include "Particle/ParticleEmitters.h"
#include "Particle/Effect.h"

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





#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};