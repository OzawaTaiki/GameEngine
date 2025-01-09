#pragma once

#include <Rendering/Light/LightGroup.h>

#include <wrl.h>
#include <d3d12.h>

class LightingSystem
{
public:

    static LightingSystem* GetInstance();

    void Initialize();

    void QueueCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index);

    void SetLightGroup(LightGroup* _lightGroup) { *lightData_ = _lightGroup->GetLightData(); }


private:

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