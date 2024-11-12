#include "SkyDome.h"

void SkyDome::Initialize()
{
    model_ = Model::CreateFromObj("skydome/skydome.obj");
    model_->SetLightingFlag(false);
    worldTransform_.Initialize();
    worldTransform_.scale_ = { 500,500,500 };
}

void SkyDome::Update()
{
    worldTransform_.UpdateData();
}

void SkyDome::Draw(const Camera* camera)
{
    model_->Draw(worldTransform_, camera);
}
