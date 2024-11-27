#pragma once
#include "BaseScene.h"
#include "Camera.h"
#include "Input.h"
#include "LineDrawer.h"
#include "WorldTransform.h"
#include "../AnimationModel/AnimationModel.h"
#include "ObjectModel.h"
#include "Audio.h"
#include "ObjectColor.h"
#include "ParticleManager.h"
#include "ParticleEmitters.h"
#include "ConfigManager.h"
#include "DebugCamera.h"

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