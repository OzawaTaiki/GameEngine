#pragma once

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RenderTexture.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/PostEffects/PostEffectBase.h>

#include <memory>
#include <string>
#include <cstdint>


using LayerID = int32_t;


namespace Engine {

class PostEffectBase;
class LayerSystem
{
private:
    struct EffectInfo
    {
        std::string outputTexture;  // このエフェクトの出力先テクスチャ名
    };

    struct LayerInfo
    {
        std::string name;
        int32_t priority;
        bool enabled;

        RenderTarget* renderTarget;
        PSOFlags::BlendMode blendMode;

        bool hasEffect;
        std::vector<EffectInfo> effectChain;  // エフェクトのチェーン
        std::string finalEffectOutput;         // 最終的な出力テクスチャ名

        uint32_t uavIndex;
        bool isOutputLayer;
    };


public:
    LayerSystem() = default;
    ~LayerSystem() = default;

    // 初期化
    static void Initialize();

    // レイヤーを作成する
    static LayerID CreateLayer(const std::string& layerName, int32_t _priority, PSOFlags::BlendMode _blendmode = PSOFlags::BlendMode::Normal);

    static LayerID CreateOutputLayer(const std::string& layerName);

    static uint32_t GetUAVIndex(const std::string& layerName);

    // レイヤーを設定する
    static void SetLayer(LayerID layerID);

    // レイヤー名で設定する
    static void SetLayer(const std::string& layerName);

    static LayerID GetCurrentLayerID();

    static void ApplyPostEffect(const std::string& _sourceLayerName,
                                const std::string& _targetLayerName,
                                PostEffectBase* _effect);

    static void CompositeAllLayers(const std::string& _finalRendertextureName = "final");

    static LayerInfo& GetLayerInfo(const std::string& _layerName);

    static void Finalize();


private:

    // singleton
    static std::unique_ptr<LayerSystem> instance_;

    LayerID currentLayerID_ = 0;

    std::unordered_map<LayerID, LayerInfo> layerInfos_;
    std::unordered_map<std::string, LayerID> nameToID_;

private:

    LayerSystem(const LayerSystem&) = delete;
    LayerSystem& operator=(const LayerSystem&) = delete;

};

} // namespace Engine
