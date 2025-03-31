#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <memory>

class JsonBinder;
class CollisionLayerManager
{
public:
    // インスタンスを取得する
    static CollisionLayerManager* GetInstance();
    // Layerを取得する
    uint32_t GetLayer(const std::string& _layer);

    void InitJsonBinder();

private:

    // Layerのマップ
    std::unordered_map<std::string, uint32_t> layerMap_;
    std::vector<std::string> layerNames_;

    std::unique_ptr<JsonBinder> jsonBinder_;


private:
    CollisionLayerManager();
    ~CollisionLayerManager();
    CollisionLayerManager(const CollisionLayerManager&) = delete;
    CollisionLayerManager& operator=(const CollisionLayerManager&) = delete;

};