#include "ObjectColor.h"
#include "DXCommon.h"

void ObjectColor::Initialize()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
    constMap_->color = { 1.0f,1.0f ,1.0f ,1.0f };
}

void ObjectColor::SetColor(const Vector4& _color)
{
    constMap_->color = _color;
}

void ObjectColor::TransferData(UINT _index, ID3D12GraphicsCommandList* _commandList)const
{
    _commandList->SetGraphicsRootConstantBufferView(_index, resource_->GetGPUVirtualAddress());
}
