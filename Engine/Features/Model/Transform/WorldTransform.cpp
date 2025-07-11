#include <Features/Model/Transform/WorldTransform.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Core/DXCommon/DXCommon.h>

void WorldTransform::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(DataForGPU));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    scale_ = { 1.0f,1.0f ,1.0f };
    rotate_ = { 0.0f,0.0f ,0.0f };
    transform_ = { 0.0f,0.0f ,0.0f };

    matWorld_ = MakeAffineMatrix(scale_, rotate_, transform_);
}

void WorldTransform::UpdateData(bool _useQuaternion)
{
    if (_useQuaternion)
        matWorld_ = MakeAffineMatrix(scale_, quaternion_, transform_);
    else
        matWorld_ = MakeAffineMatrix(scale_, rotate_, transform_);
    if (parent_)
    {
        matWorld_ *= parent_->matWorld_;
    }
    else if (parentMatrix_)
    {
        matWorld_ *= *parentMatrix_;
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

void WorldTransform::SetParent(const WorldTransform* _parent)
{
    parent_ = _parent;
    parentMatrix_ = nullptr;
}

void WorldTransform::SetParent(const Matrix4x4* _parentMatrix)
{
    parentMatrix_ = _parentMatrix;
    parent_ = nullptr;
}

void WorldTransform::UpdateData(const std::initializer_list<Matrix4x4>& _mat)
{
    Matrix4x4 matrix = MakeIdentity4x4();
    for (auto& mat : _mat)
    {
        matWorld_ = matrix * mat;
    }
    matWorld_ *= MakeAffineMatrix(scale_, rotate_, transform_);
    if (parent_)
    {
        matWorld_ *= parent_->matWorld_;
    }
    TransferData();
}

void WorldTransform::TransferData()
{
    constMap_->World = matWorld_;
    constMap_->worldInverseTranspose = Transpose(Inverse(matWorld_));
}

void WorldTransform::QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const
{
    _cmdList->SetGraphicsRootConstantBufferView(_index, resource_->GetGPUVirtualAddress());
}
