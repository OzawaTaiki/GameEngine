#pragma once

#include "../Collider/Collider.h"
#include "WorldTransform.h"
#include "Model.h"
#include "Camera.h"

class Enemy
{
public:

    Enemy() = default;
    ~Enemy() = default;

    void Initialize(const Vector3& _pos, const Vector3& _velo, float _lifeTime);
    void Update();
    void Draw(const Camera* _camera);

    void OnCollision();

private:

    WorldTransform worldTransform_ = {};
    Model* model_ = nullptr;
    std::unique_ptr<Collider> collider_ = nullptr;

    Vector3 velocity_ = {};
    float lifeTime_ = 0.0f;

    bool isAlive_ = true;

    bool IsDrawBoundingBox_ = true;

};