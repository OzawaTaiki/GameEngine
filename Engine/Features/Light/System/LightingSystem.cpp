#include <Features/Light/System/LightingSystem.h>

#include <Core/DXCommon/DXCommon.h>

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

void LightingSystem::QueueGraphicsCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index)
{
    if (lightGroup_)
        *lightData_ = lightGroup_->GetLightData();
    else
        *lightData_ = LightGroup::GetDefaultLightData();

    _commandList->SetGraphicsRootConstantBufferView(_index, lightBuffer_->GetGPUVirtualAddress());
}

void LightingSystem::QueueComputeCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index)
{
    if (lightGroup_)
        *lightData_ = lightGroup_->GetLightData();
    else
        *lightData_ = LightGroup::GetDefaultLightData();

    _commandList->SetComputeRootConstantBufferView(_index, lightBuffer_->GetGPUVirtualAddress());
}
