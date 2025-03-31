#include "CollisionLayerManager.h"

#include <Features/Json/JsonBinder.h>

CollisionLayerManager* CollisionLayerManager::GetInstance()
{
    static CollisionLayerManager instance;
    return &instance;
}

uint32_t CollisionLayerManager::GetLayer(const std::string& _layer)
{
    // Layerが登録されているか確認
    auto it = layerMap_.find(_layer);
    // 登録されている場合それを返す
    if (it != layerMap_.end())
        return it->second;

    // 登録されていない場合新たに登録する
    layerMap_[_layer] = static_cast <uint32_t>(1 << layerMap_.size());

    layerNames_.push_back(_layer);

    // 登録したLayerを返す
    uint32_t result = layerMap_[_layer];
    return result;
}

void CollisionLayerManager::InitJsonBinder()
{
    jsonBinder_ = std::make_unique<JsonBinder>("CollisionLayer", "Resources/Data/CollisionLayer/");
    jsonBinder_->RegisterVariable("layerNames", &layerNames_);
}

CollisionLayerManager::CollisionLayerManager()
{
    InitJsonBinder();
}

CollisionLayerManager::~CollisionLayerManager()
{
    jsonBinder_->Save();

    jsonBinder_.reset();
}
