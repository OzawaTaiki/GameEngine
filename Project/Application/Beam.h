#pragma once

#include "Model.h"
#include "WorldTransform.h"
#include "Camera.h"

class Beam
{
public:

    Beam() = default;
    ~Beam() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

    void SetTarget(const Vector3& _target) { target_ = _target; }
    void SetParent(const WorldTransform* _parent) { worldTransform_.parent_ = _parent; }

private:


    Model* model_ = nullptr;
    WorldTransform worldTransform_;

    Vector3 target_ = {};

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};