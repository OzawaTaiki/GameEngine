#include "LightingSystem.h"

#include <Core/DirectX/DXCommon.h>

LightingSystem* LightingSystem::GetInstance()
{
    static LightingSystem instance;
    return &instance;
}

void LightingSystem::Initialize()
{
    lightBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(LightGroup::LightTransferData));
    lightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

    *lightData_ = LightGroup::GetDefaultLightData();
}

void LightingSystem::QueueCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index)
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootConstantBufferView(_index, lightBuffer_->GetGPUVirtualAddress());

}
