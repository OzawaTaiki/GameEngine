#pragma once

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RenderTexture.h>

#include <Features/PostEffects/PostEffectBase.h>

#include <memory>
#include <string>
#include <cstdint>


using LayerID = int32_t;

class PostEffectBase;
class LayerSystem
{
public:

    // 初期化
    static void Initialize();

    // レイヤーを作成する
    static LayerID CreateLayer(const std::string& layerName, int32_t _priority);

    // レイヤーを設定する
    static void SetLayer(LayerID layerID);

    // レイヤー名で設定する
    static void SetLayer(const std::string& layerName);

    static void ApplyPostEffect(const std::string& _sourceLayerName,
        const std::string& _targetLayerName,
        PostEffectBase* _effect,
        PostEffectBaseData* _effectData = nullptr);

    static void CompositeAllLayers(const std::string& _finalRendertextureName = "final");

    static void Finalize();

private:
    struct LayerInfo
    {
        std::string name;
        int32_t priority; // レイヤーの優先度
        bool enabled; // レイヤーが有効かどうか

        RenderTarget* renderTarget;

        bool hasEffect = false; // エフェクトがあるかどうか
        std::string effectOutputTexture;

    };

private:



    // singleton
    static std::unique_ptr<LayerSystem> instance_;

    std::unordered_map<LayerID, LayerInfo> layerInfos_;
    std::unordered_map<std::string, LayerID> nameToID_;


private:

    LayerSystem() = default;
    ~LayerSystem() = default;
    LayerSystem(const LayerSystem&) = delete;
    LayerSystem& operator=(const LayerSystem&) = delete;

};