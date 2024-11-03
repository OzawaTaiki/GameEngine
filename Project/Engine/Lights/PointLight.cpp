#include "PointLight.h"
#include "DXCommon.h"

void PointLight::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    color_ = { 1.0f,1.0f ,1.0f ,1.0f };
    position_ = { 0.0f,2.0f,0.0f };
    intendity_ = 1.0f;
    radius_ = 5.0f;
    decay_ = 0.5f;
    useHalfLambert_ = true;
}

void PointLight::Update()
{
}

void PointLight::Draw()
{
}

void PointLight::TransferData()
{
    bool flag = static_cast<bool>(useHalfLambert_);
    *constMap_ = { color_,position_,intendity_,radius_,decay_, flag };
}
