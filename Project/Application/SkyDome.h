#pragma once

#include "WorldTransform.h"
#include "Model.h"
#include "Camera.h"

class SkyDome
{
public:
    SkyDome() = default;
    ~SkyDome() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* camera);

private:
    Model* model_ = nullptr;
    WorldTransform worldTransform_ = {};

};