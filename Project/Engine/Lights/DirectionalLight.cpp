#include "DirectionalLight.h"
#include "DXCommon.h"
#include "VectorFunction.h"
void DirectionalLight::Initialize()
{
    resource_= DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    color_ = { 1.0f,1.0f ,1.0f ,1.0f };
    direction_ = { 0.0f,-1.0f,0.0f };
    intendity_ = 1.0f;
    useHalfLambert_ = true;

}

void DirectionalLight::Update()
{
}

void DirectionalLight::Draw()
{
}

void DirectionalLight::TransferData()
{
    bool flag = static_cast<bool>(useHalfLambert_);
    *constMap_ = { color_,direction_,intendity_,flag };


}

void DirectionalLight::SetDirection(const Vector3& _directoin)
{
    direction_ = Normalize(_directoin);
}
