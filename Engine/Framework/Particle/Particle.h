#pragma once

#include <Rendering/Model/Model.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Physics/Math/Matrix4x4.h>
#include "ParticleInitParam.h"

class Particle
{
public:

    Particle() = default;
    ~Particle() = default;

    void Initialize(const ParticleInitParam& _param);
    void Update();
    void Draw();

    bool IsAlive() const { return isAlive_; }

    Matrix4x4 GetWorldMatrix() const { return matWorld_; }
    Vector3 GetPosition() const { return translate_; }
    Vector3 GetRotation() const { return rotation_; }
    Vector3 GetScale() const { return scale_; }
    Vector4 GetColor() const { return color_; }
    Matrix4x4 GetDirectionMatrix() const { return directionMatrix_; }

private:

    ParticleInitParam parameter_;

    bool isAlive_ = true;

    float currentTime_ = 0;
    float t_ = 0;

    Vector3 velocity_ = {};
    float lifeTime_ = 1.0f;

    Vector3 translate_ = { 0.0f,0.0f ,0.0f };
    Vector3 rotation_ = { 0.0f,0.0f ,0.0f };
    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector4 color_ = { 1.0f,1.0f ,1.0f,1.0f };
    float speed_ = 0.0f;
    Vector3 direction_ = {};
    Vector3 acceleration_ = {};

    float deceleration_ = 0.0f;

    Matrix4x4 matWorld_ = Matrix4x4::Identity();
    Matrix4x4 directionMatrix_ = Matrix4x4::Identity();



};
