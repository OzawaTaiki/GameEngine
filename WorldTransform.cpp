#include "WorldTransform.h"
#include "MatrixFunction.h"
#include "DXCommon.h"

void WorldTransform::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(DataForGPU));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}

void WorldTransform::TransferData(const Matrix4x4& _viewProjectoin)
{
    constMap_->WVP = MakeAffineMatrix(scale_, rotate_, transform_);
    constMap_->World = constMap_->WVP * _viewProjectoin;
    constMap_->worldInverseTranspose = Transpose(Inverse(constMap_->World));
}