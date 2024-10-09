#include "SpotLight.h"
#include "DXCommon.h"
#include "VectorFunction.h"
#include <numbers>
#include <cmath>

void SpotLight::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    color_ = { 1.0f,1.0f ,1.0f ,1.0f };
    position_ = { 0.0f,2.0f,0.0f };
    direction_ = Normalize({ -1.0f,1.0f ,0.0f });
    intensity_ = 4.0f;
    distance_ = 7.0f;
    cosAngle_ = std::cosf(std::numbers::pi_v<float> / 3.0f);
    falloutStartAngle_= std::cos(std::numbers::pi_v<float> / 3.0f);
    decay_ = 2.0f;
    useHalfLambert_ = true;

}

void SpotLight::Update()
{
}

void SpotLight::Draw()
{
}

void SpotLight::TransferData()
{
    bool flag = static_cast<bool>(useHalfLambert_);
    *constMap_ = { color_,position_,intensity_,direction_,distance_,decay_, cosAngle_,falloutStartAngle_,flag };
}
