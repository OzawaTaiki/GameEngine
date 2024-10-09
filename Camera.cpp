#include "Camera.h"
#include "MatrixFunction.h"
#include "DXCommon.h"

void Camera::Initialize()
{
    Map();
    TransferData();
}

void Camera::Update()
{
}

void Camera::Draw()
{
}

void Camera::TransferData()
{
    matWorld_ = MakeAffineMatrix(scale_, rotate_, transform_);
    matView_ = Inverse(matWorld_);
    matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
    matViewProjection_ = matView_ * matProjection_;

    *constMap_ = transform_;
}

void Camera::Map()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(Vector3));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
    *constMap_ = transform_;
}
