#pragma once

#include "Camera.h"

#include <memory>

class RailCamera
{
public:

    void Initialize();
    void Update();
    void SetParent(const WorldTransform* _parent) { camera_->SetParent(_parent); }

    Matrix4x4 GetViewMatrix() const{ return camera_->matView_; }
    Matrix4x4 GetViewProjection() const { return camera_->GetViewProjection(); }

private:

    std::unique_ptr<Camera> camera_ = nullptr;
};