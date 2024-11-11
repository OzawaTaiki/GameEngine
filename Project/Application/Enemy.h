#pragma once

#include "Collider.h"
#include "WorldTransform.h"
#include "Model.h"
#include "Camera.h"

class Enemy : private Collider
{
public:

    Enemy() = default;
    ~Enemy() = default;

    void Initialize(const Vector3& _pos);
    void Update();
    void Draw(const Camera* _camera);

    Vector3 GetWorldPosition() const override{ return worldTransform_.GetWorldPosition(); }
    void OnCollision();

private:

    WorldTransform worldTransform_ = {};
    Model* model_ = nullptr;

    bool isAlive_ = true;

    bool IsDrawBoundingBox_ = true;

};