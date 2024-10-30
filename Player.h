#pragma once

//#include "Vector3.h"

#include "Model.h"
#include "WorldTransform.h"

#include <memory>

class Player
{
public:

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

private:

    WorldTransform worldTransform_;
    Model* model_ = nullptr;




};