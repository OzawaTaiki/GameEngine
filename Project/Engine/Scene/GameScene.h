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
    std::unique_ptr<Audio> audio_;
    uint32_t handle_;

    ObjectModel* model_;
    AnimationModel* humanModel_;
    WorldTransform trans_;
    ObjectColor* color_;

    ParticleEmitter* emit_;
};