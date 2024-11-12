#include "RailCamera.h"

void RailCamera::Initialize()
{
    camera_ = std::make_unique<Camera>();
    camera_->Initialize();
    camera_->translate_ = { 0,1.25f,-0.65f };
}

void RailCamera::Update()
{
    camera_->Update();
    camera_->UpdateMatrix();
    camera_->TransferData();
}
