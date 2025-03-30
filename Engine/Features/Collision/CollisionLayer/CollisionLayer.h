#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

class JsonBinder;
class CollisionLayer
{
public:

    // コンストラクタ
    CollisionLayer();

    // 自身のLayerを取得する
    uint32_t GetLayer() const { return layer_; }

    // 衝突しないLayerを取得する
    uint32_t GetLayerMask() const { return layerMask_; }

    // LayerMaskを取得する
    void SetLayer(const std::string& _layer);

    // LayerMaskを設定する
    void SetLayerMask(const std::string& _layer);

    // jsonBinderへの登録
    void RegisterLayer(JsonBinder* _jsonBinder);

private:

    // 自身のLayer
    uint32_t layer_ = 0;

    // 衝突しないLayer
    uint32_t layerMask_ = 0;

};