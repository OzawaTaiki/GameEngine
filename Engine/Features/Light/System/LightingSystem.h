#pragma once

#include <Features/Light/Group/LightGroup.h>

#include <wrl.h>
#include <d3d12.h>

class LightingSystem
{
public:

    static LightingSystem* GetInstance();

    void Initialize();

    void Update();

    void SetActiveGroup(std::shared_ptr<LightGroup> _lightGroup) { activeGroup_ = _lightGroup; }
    std::shared_ptr<LightGroup> GetLightGroup() { return activeGroup_.lock(); }

    void QueueGraphicsCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index);
    void QueueComputeCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index);

    void QueuePointLightShadowCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index, PointLightComponent* _light);
    void QueueSpotLightShadowCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index, SpotLightComponent* _light);

    void CreatePointLightShadowMap(const std::string& _lightName);
    void CreateSpotLightShadowMap(const std::string& _lightName);

    void SetShadowMapSize(uint32_t _size);
    uint32_t GetShadowMapSize() const { return shadowMapSize_; }


private:

    Microsoft::WRL::ComPtr<ID3D12Resource> lightBuffer_ = nullptr;
    LightGroup::LightTransferData* lightData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> shadowPointLightBuffer_ = nullptr;
    PointLight* shadowPointLightData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> shadowSpotLightBuffer_ = nullptr;
    SpotLight* shadowSpotLightData_ = nullptr;

    std::weak_ptr<LightGroup> activeGroup_;

    uint32_t shadowMapSize_ = 1024;

    uint32_t pointLightShadowMapHandle = 0;

private: // コピー禁止
    LightingSystem() = default;
    ~LightingSystem() = default;
    LightingSystem(const LightingSystem&) = delete;
    LightingSystem& operator=(const LightingSystem&) = delete;
    LightingSystem(LightingSystem&&) = delete;
    LightingSystem& operator=(LightingSystem&&) = delete;

};
