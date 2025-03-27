#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

class CollisionLayerManager
{
public:
    // インスタンスを取得する
    static CollisionLayerManager* GetInstance();
    // Layerを取得する
    uint32_t GetLayer(const std::string& _layer);

private:

    // Layerのマップ
    std::unordered_map<std::string, uint32_t> layerMap_;

private:
    CollisionLayerManager();
    ~CollisionLayerManager() = default;
    CollisionLayerManager(const CollisionLayerManager&) = delete;
    CollisionLayerManager& operator=(const CollisionLayerManager&) = delete;

};