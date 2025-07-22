#include "LayerSystem.h"

#include <Core/DXCommon/RTV/RTVManager.h>
#include <Core/WinApp/WinApp.h>

std::unique_ptr<LayerSystem> LayerSystem::instance_ = nullptr;

void LayerSystem::Initialize()
{
    if(!instance_)
        instance_ = std::make_unique<LayerSystem>();

    instance_->layerInfos_.clear();
    instance_->nameToID_.clear();

    RTVManager::GetInstance()->
        CreateRenderTarget("final", WinApp::kWindowWidth_, WinApp::kWindowHeight_,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.0190f, 0.0190f, 0.0933f, 1.0f), false);
}

LayerID LayerSystem::CreateLayer(const std::string& layerName, int32_t _priority)
{
    if (!instance_)
        Initialize();

    for (const auto& [id, info] : instance_->layerInfos_)
    {
        if (info.name == layerName)
        {
            return id;
        }
    }

        // レイヤーのRenderTargetを作成
    LayerID layerID = RTVManager::GetInstance()->
        CreateRenderTarget(layerName, WinApp::kWindowWidth_, WinApp::kWindowHeight_,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.0190f, 0.0190f, 0.0933f, 1.0f), false);

    instance_->layerInfos_.emplace(layerID, LayerInfo{ layerName, _priority, true, nullptr });
    instance_->layerInfos_[layerID].renderTarget = RTVManager::GetInstance()->GetRenderTexture(layerName);

    instance_->nameToID_[layerName] = layerID;

    return layerID;
}

void LayerSystem::SetLayer(LayerID _layerID)
{
    if (!instance_)
        Initialize();

    auto it = instance_->layerInfos_.find(_layerID);
    if (it != instance_->layerInfos_.end())
    {
        RTVManager::GetInstance()->SetRenderTexture(_layerID);
    }
    else
    {
        // レイヤーが存在しない場合のエラーハンドリング
        // 例: エラーメッセージを出力する、デフォルトのレイヤーに戻すなど
    }
}

void LayerSystem::SetLayer(const std::string& layerName)
{
    if (!instance_)
        Initialize();

    auto it = instance_->nameToID_.find(layerName);
    if (it != instance_->nameToID_.end())
    {
        SetLayer(it->second);  // LayerID版のSetLayerを呼び出し
    }

}

void LayerSystem::ApplyPostEffect(const std::string& _sourceLayerName, const std::string& _targetLayerName, PostEffectBase* _effect)
{
    if (!instance_)
        Initialize();

    _effect->Apply(_sourceLayerName, _targetLayerName);

    auto it = instance_->nameToID_.find(_sourceLayerName);
    if (it != instance_->nameToID_.end())
    {
        LayerID sourceLayerID = it->second;
        auto& sourceLayerInfo = instance_->layerInfos_[sourceLayerID];

        // エフェクトの出力先を設定
        sourceLayerInfo.effectOutputTexture = _targetLayerName;
        sourceLayerInfo.hasEffect = true;
    }
    else
    {
        // ソースレイヤーが存在しない場合のエラーハンドリング

    }

}

void LayerSystem::CompositeAllLayers(const std::string& _finalRendertextureName)
{
    if (!instance_)
        Initialize();

    // priority順でソート
    std::vector<std::pair<LayerID, LayerInfo*>> sortedLayers;
    for (auto& [id, info] : instance_->layerInfos_)
    {
        if (info.enabled)
        {
            sortedLayers.push_back({ id, &info });
        }
    }

    std::sort(sortedLayers.begin(), sortedLayers.end(),
        [](const auto& a, const auto& b){
            return a.second->priority < b.second->priority;
        });

    RTVManager::GetInstance()->SetRenderTexture(_finalRendertextureName);
    // 全てのレイヤーを合成
    for (auto& [id, info] : sortedLayers)
    {
        if (info->enabled && info->renderTarget)
        {
            std::string textureToUse = info->hasEffect ? info->effectOutputTexture : info->name;

            RTVManager::GetInstance()->DrawRenderTexture(textureToUse);
        }
    }
}

void LayerSystem::Finalize()
{
    if (instance_)
    {
        instance_->layerInfos_.clear();
        instance_->nameToID_.clear();
        instance_.reset();
    }
}
