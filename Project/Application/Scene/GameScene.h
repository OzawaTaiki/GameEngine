#pragma once
#include "eScene/BaseScene.h"

#include "Camera/Camera.h"
#include "Camera/DebugCamera.h"
#include "Input/Input.h"
#include "LineDrawer/LineDrawer.h"
#include "Model/WorldTransform.h"
#include "Model/AnimationModel.h"
#include "Model/ObjectModel.h"
#include "Model/ObjectColor.h"
#include "Audio/Audio.h"
#include "Particle/ParticleManager.h"
#include "Particle/ParticleEmitters.h"

#include <memory>

class GameScene : public BaseScene
{
public:

    // Factory Method
    static std::unique_ptr<BaseScene> Create();

    ~GameScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;


private:
    Input* input_ = nullptr;
    LineDrawer* lineDrawer_ = nullptr;
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
    bool activeDebugCamera_ = false;
    std::unique_ptr<Audio> audio_;
    uint32_t handle_;


};