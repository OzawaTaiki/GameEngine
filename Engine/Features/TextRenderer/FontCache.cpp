#include "FontCache.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>

#include <fstream>
#include <cassert>



namespace Engine {

FontCache* FontCache::GetInstance()
{
    static FontCache instance;
    return &instance;
}

void FontCache::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const Vector2& _windowSize)
{
    if (!_device || !_cmdList)
        return;

    device_ = _device;
    cmdList_ = _cmdList;

    windowSize_ = _windowSize;

    dxCommon_ = DXCommon::GetInstance();
    srvManager_ = SRVManager::GetInstance();

    FontConfig config = {};

    auto atlasData = std::make_unique<AtlasData>();
    atlasData->Initialize(device_, cmdList_, config.fontFilePath, config.fontSize, config.atlasSize);

    atlasDataMap_.emplace(std::make_pair(config.fontFilePath, config.fontSize), std::move(atlasData));
}

AtlasData* FontCache::GetAtlasData(const std::string& _fontFilePath, float _fontSize)
{
    auto pair = std::make_pair(_fontFilePath, _fontSize);

    auto it = atlasDataMap_.find(pair);
    if (it != atlasDataMap_.end())
    {
        return it->second.get();
    }

    // 存在しない場合は新しく作成
    auto atlasData = std::make_unique<AtlasData>();
    atlasData->Initialize(device_, cmdList_,  _fontFilePath, _fontSize, { 4096, 4096 });

    AtlasData* atlasPtr = atlasData.get();

    atlasDataMap_.emplace(pair, std::move(atlasData));

    return atlasPtr;

}

} // namespace Engine
