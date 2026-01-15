#include <Features/Light/System/LightingSystem.h>

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RTVManager.h>


namespace Engine {

LightingSystem* LightingSystem::GetInstance()
{
    static LightingSystem instance;
    return &instance;
}

void LightingSystem::Initialize()
{
    auto dxCommon = DXCommon::GetInstance();

    // メインのライトバッファを作成
    lightBuffer_ = dxCommon->CreateBufferResource(sizeof(LightGroup::LightTransferData));
    lightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

    // シャドウマップ用のポイントライトバッファを作成
    shadowPointLightBuffer_ = dxCommon->CreateBufferResource(sizeof(PointLight));
    shadowPointLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&shadowPointLightData_));

    // シャドウマップ用のスポットライトバッファを作成
    shadowSpotLightBuffer_ = dxCommon->CreateBufferResource(sizeof(SpotLight));
    shadowSpotLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&shadowSpotLightData_));

    // デフォルト値を設定
    *lightData_ = {}; // 0で初期化
    *shadowPointLightData_ = {}; // 0で初期化
    *shadowSpotLightData_ = {}; // 0で初期化

    // シャドウマップのサイズを設定
    shadowMapSize_ = 128;
    if (LightGroup::GetShadowMapSize() != shadowMapSize_) {
        LightGroup::SetShadowMapSize(shadowMapSize_);
    }

}

void LightingSystem::QueueGraphicsCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index)
{
    // アクティブなライトグループからデータを取得して転送
    if (auto group = activeGroup_.lock())
    {
        *lightData_ = group->GetLightData();
    }
    else {
        // デフォルト値を設定
        *lightData_ = {};
        lightData_->directionalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        lightData_->directionalLight.direction = { 0.0f, -1.0f, 0.0f };
        lightData_->directionalLight.intensity = 1.0f;
    }

    // コマンドリストにセット
    _commandList->SetGraphicsRootConstantBufferView(_index, lightBuffer_->GetGPUVirtualAddress());
}

void LightingSystem::QueueComputeCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index)
{
    // アクティブなライトグループからデータを取得して転送
    if (auto group = activeGroup_.lock()) {
        *lightData_ = group->GetLightData();
    }
    else {
        // デフォルト値を設定
        *lightData_ = {};
        lightData_->directionalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        lightData_->directionalLight.direction = { 0.0f, -1.0f, 0.0f };
        lightData_->directionalLight.intensity = 1.0f;
    }
    // コマンドリストにセット
    _commandList->SetComputeRootConstantBufferView(_index, lightBuffer_->GetGPUVirtualAddress());
}

void LightingSystem::QueuePointLightShadowCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index, PointLightComponent* _light)
{
    if (!_light) return;

    // ポイントライトのデータを取得してシャドウマップ用バッファにコピー
    *shadowPointLightData_ = _light->GetData();

    // コマンドリストにセット
    _commandList->SetGraphicsRootConstantBufferView(_index, shadowPointLightBuffer_->GetGPUVirtualAddress());
}

void LightingSystem::QueueSpotLightShadowCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index, SpotLightComponent* _light)
{
    if (!_light) return;

    // スポットライトのデータを取得してシャドウマップ用バッファにコピー
    *shadowSpotLightData_ = _light->GetData();

    // コマンドリストにセット
    _commandList->SetGraphicsRootConstantBufferView(_index, shadowSpotLightBuffer_->GetGPUVirtualAddress());
}

void LightingSystem::CreatePointLightShadowMap(const std::string& _lightName)
{
    auto group = activeGroup_.lock();
    if (!group) return;

    // 指定された名前のポイントライトを取得
    auto pointLight = group->GetPointLight(_lightName);

    if (!pointLight || !pointLight->IsCastShadow())
        return;

    // シャドウマップの作成
    pointLight->CreateShadowMaps(shadowMapSize_);
}

void LightingSystem::CreateSpotLightShadowMap(const std::string& _lightName)
{
    auto group = activeGroup_.lock();
    if (!group) return;

    // 指定された名前のスポットライトを取得
    auto spotLight = group->GetSpotLight(_lightName);

    if (!spotLight || !spotLight->IsCastShadow())
        return;

    // シャドウマップの作成
    spotLight->CreateShadowMap(shadowMapSize_);
}

void LightingSystem::SetShadowMapSize(uint32_t _size)
{
    shadowMapSize_ = _size;
    LightGroup::SetShadowMapSize(_size);

    // 既存のすべてのシャドウマップを再生成
    auto group = activeGroup_.lock();
    if (group)
    {// ディレクショナルライトのシャドウマップを更新
        auto dirLight = group->GetDirectionalLight();
        if (dirLight && dirLight->IsCastShadow()) {
            dirLight->UpdateViewProjection();
        }

        // ポイントライトのシャドウマップを更新
        auto pointLights = group->GetAllPointLights();
        for (auto& light : pointLights) {
            if (light->IsCastShadow()) {
                light->CreateShadowMaps(shadowMapSize_);
            }
        }

        // スポットライトのシャドウマップを更新
        auto spotLights = group->GetAllSpotLights();
        for (auto& light : spotLights) {
            if (light->IsCastShadow()) {
                light->CreateShadowMap(shadowMapSize_);
            }
        }
    }
}

} // namespace Engine
