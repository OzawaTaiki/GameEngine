#pragma once

#include "Model.h"
#include "WorldTransform.h"
#include "Camera.h"
#include "../Collider/Collider.h"

class Beam 
{
public:

    Beam() = default;
    ~Beam() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

    void OnCollision() {};

    void SetTarget(const Vector3& _target) { target_ = _target; }
    void SetParent(const WorldTransform* _parent) { worldTransform_.parent_ = _parent; }


private:

    std::unique_ptr<Collider> collider_ = nullptr;

    Model* model_ = nullptr;
    WorldTransform worldTransform_;

    Vector3 target_ = {};

    bool isDrawBoundingBox_ = true;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};