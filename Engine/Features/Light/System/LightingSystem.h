#pragma once

#include <Features/Light/Group/LightGroup.h>

#include <wrl.h>
#include <d3d12.h>

class LightingSystem
{
public:

    static LightingSystem* GetInstance();

    void Initialize();

    void QueueGraphicsCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index);
    void QueueComputeCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index);

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = _lightGroup; }


private:

    LightGroup* lightGroup_ = nullptr;
    LightGroup::LightTransferData* lightData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> lightBuffer_ = nullptr;

private: // コピー禁止
    LightingSystem() = default;
    ~LightingSystem() = default;
    LightingSystem(const LightingSystem&) = delete;
    LightingSystem& operator=(const LightingSystem&) = delete;
    LightingSystem(LightingSystem&&) = delete;
    LightingSystem& operator=(LightingSystem&&) = delete;

};
