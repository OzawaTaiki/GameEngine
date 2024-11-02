#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Input.h"
#include "LineDrawer.h"
#include "WorldTransform.h"
#include "Model.h"
#include "Audio.h"
#include "ObjectColor.h"
#include "ParticleManager.h"
#include "ParticleEmitters.h"

#include <memory>

class GameScene : public Scene
{
public:
    ~GameScene();

    void Initialize();
    void Update();
    void Draw();


private:
    Input* input_ = nullptr;
    LineDrawer* lineDrawer_ = nullptr;
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<Audio> audio_;
    uint32_t handle_;

    Model* model_;
    WorldTransform trans_;
    ObjectColor* color;

    ParticleEmitter* emit_;
};