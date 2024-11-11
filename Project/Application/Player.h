#pragma once

//#include "Vector3.h"

#include "Model.h"
#include "WorldTransform.h"
#include "Camera.h"
#include "Beam.h"

#include <memory>

class Player
{
public:

    void Initialize();
    void Update(const Matrix4x4& _matVP);
    void Draw(const Camera* _camera);

    WorldTransform* GetWorldTransform() { return &worldTransform_; }

private:

    void Aim(const Matrix4x4& _matVP);

    WorldTransform worldTransform_;
    Model* model_ = nullptr;

    std::unique_ptr<Beam> beam_ = nullptr;
    bool isBeamActive_ = false;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};
