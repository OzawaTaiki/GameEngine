#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Input.h"

#include <memory>

class GameScene : public Scene
{
public:
    void Initialize();
    void Update();
    void Draw();


private:
    Input* input_ = nullptr;
    std::unique_ptr<Camera> camera_ = nullptr;

};