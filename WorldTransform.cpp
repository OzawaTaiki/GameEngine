#include "WorldTransform.h"
#include "MatrixFunction.h"
#include "DXCommon.h"

void WorldTransform::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(DataForGPU));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    scale_ = { 1.0f,1.0f ,1.0f };
    rotate_ = { 0.0f,0.0f ,0.0f };
    transform_ = { 0.0f,0.0f ,0.0f };

    matWorld_ = MakeAffineMatrix(scale_, rotate_, transform_);
}

void WorldTransform::TransferData(const Matrix4x4& _viewProjectoin)
{
    constMap_->World = matWorld_;
    constMap_->WVP = constMap_->World * _viewProjectoin;
    constMap_->worldInverseTranspose = Transpose(Inverse(constMap_->World));
}
