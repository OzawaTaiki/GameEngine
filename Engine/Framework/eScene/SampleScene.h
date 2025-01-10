#pragma once
#include "BaseScene.h"
#include <Framework/Camera/Camera.h>
#include <Framework/Camera/DebugCamera.h>
#include <Rendering/Model/ObjectModel.h>
#include <Rendering/Model/AnimationModel.h>

#include <Framework/Particle/ParticleManager.h>
#include <Rendering/LineDrawer/LineDrawer.h>
#include <Systems/Input/Input.h>
#include <Systems/Time/GameTime.h>

#include <memory>

class SampleScene : public BaseScene
{
public:

    static std::unique_ptr<BaseScene> Create();

     ~SampleScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:
    // シーン関連
    Camera SceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleManager* particleManager_ = nullptr;

    std::unique_ptr<AnimationModel> aModel_ = nullptr;
    std::unique_ptr<ObjectModel> oModel_ = nullptr;

    std::unique_ptr<ObjectModel> plane_ = nullptr;

    std::unique_ptr<LightGroup> lights_;

    GameTime* gameTime_;


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
