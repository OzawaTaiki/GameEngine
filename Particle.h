#pragma once

#include "Model.h"

class Particle
{
public:

    Particle() = default;
    ~Particle() = default;

    void Initialize();
    void Update();
    void Draw();

    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotation_ = { 0.0f,0.0f ,0.0f };
    Vector3 translate_ = { 1.0f,1.0f ,1.0f };

    Vector4 color_ = { 1.0f,1.0f ,1.0f,1.0f };

private:



};