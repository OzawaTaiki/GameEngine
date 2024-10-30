#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

#include <memory>

class DebugCamera
{
public:

    void Initialize();
    void Update();

    void GetViewProjection() const;

    Vector3 translate_ = { 0,5,-20 };
    Vector3 scale_ = { 1,1,1 };
    Vector3 rotation_ = { 0.26f,0,0 };

    Matrix4x4 matView_ = {};

private:


};
