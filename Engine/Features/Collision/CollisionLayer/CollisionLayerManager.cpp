#include "CollisionLayerManager.h"

#include <Features/Json/JsonBinder.h>

CollisionLayerManager* CollisionLayerManager::GetInstance()
{
    static CollisionLayerManager instance;
    return &instance;
}

uint32_t CollisionLayerManager::GetLayer(const std::string& _layer)
{
    std::string normalizedLayer = NormalizeKey(_layer);
    // Layerが登録されているか確認
    auto it = layerMap_.find(normalizedLayer);
    // 登録されている場合それを返す
    if (it != layerMap_.end())
        return it->second;

    // 登録されていない場合新たに登録する
    layerMap_[normalizedLayer] = static_cast <uint32_t>(1 << layerMap_.size());

    layerNames_.push_back(normalizedLayer);

    // 登録したLayerを返す
    uint32_t result = layerMap_[normalizedLayer];
    return result;
}

const std::string& CollisionLayerManager::GetLayerStr(uint32_t _layer) const
{
    // Layerが登録されているか確認
    for (const auto& pair : layerMap_)
    {
        if (pair.second == _layer)
        {
            return pair.first;
        }
    }
    // 登録されていない場合は空文字列を返す
    static const std::string emptyString = "Unknown";
    return emptyString;
}

void CollisionLayerManager::InitJsonBinder()
{
    jsonBinder_ = std::make_unique<JsonBinder>("CollisionLayer", "Resources/Data/CollisionLayer/");
    jsonBinder_->RegisterVariable("layerNames", &layerNames_);
}

CollisionLayerManager::CollisionLayerManager()
{
    InitJsonBinder();
    uint32_t index = 0;
    for (const std::string& name : layerNames_)
    {
        layerMap_[name] = 1 << index++;
    }
}

CollisionLayerManager::~CollisionLayerManager()
{
    jsonBinder_->Save();

    jsonBinder_.reset();
}
