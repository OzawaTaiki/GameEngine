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

void WorldTransform::UpdateData()
{
    matWorld_ = MakeAffineMatrix(scale_, rotate_, transform_);
    if (parent_)
    {
        matWorld_ *= parent_->matWorld_;
    }
    TransferData();
}

Vector3 WorldTransform::GetWorldPosition() const
{
    Vector3 wPos;
    wPos.x = matWorld_.m[3][0];
    wPos.y = matWorld_.m[3][1];
    wPos.z = matWorld_.m[3][2];
    return wPos;
}

void WorldTransform::TransferData()
{
    constMap_->World = matWorld_;
    constMap_->worldInverseTranspose = Transpose(Inverse(matWorld_));
}
