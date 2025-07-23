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
private:
    struct LayerInfo
    {
        std::string name;
        int32_t priority; // レイヤーの優先度
        bool enabled; // レイヤーが有効かどうか

        RenderTarget* renderTarget;

        bool hasEffect = false; // エフェクトがあるかどうか
        std::string effectOutputTexture;

        uint32_t uavIndex;
        bool isOutputLayer = false; // 出力レイヤーかどうか

    };

public:
    LayerSystem() = default;
    ~LayerSystem() = default;

    // 初期化
    static void Initialize();

    // レイヤーを作成する
    static LayerID CreateLayer(const std::string& layerName, int32_t _priority);

    static LayerID CreateOutputLayer(const std::string& layerName);

    static uint32_t GetUAVIndex(const std::string& layerName);

    // レイヤーを設定する
    static void SetLayer(LayerID layerID);

    // レイヤー名で設定する
    static void SetLayer(const std::string& layerName);

    static void ApplyPostEffect(const std::string& _sourceLayerName,
        const std::string& _targetLayerName,
        PostEffectBase* _effect);

    static void CompositeAllLayers(const std::string& _finalRendertextureName = "final");

    static LayerInfo& GetLayerInfo(const std::string& _layerName);

    static void Finalize();


private:



    // singleton
    static std::unique_ptr<LayerSystem> instance_;

    std::unordered_map<LayerID, LayerInfo> layerInfos_;
    std::unordered_map<std::string, LayerID> nameToID_;


private:

    LayerSystem(const LayerSystem&) = delete;
    LayerSystem& operator=(const LayerSystem&) = delete;

};