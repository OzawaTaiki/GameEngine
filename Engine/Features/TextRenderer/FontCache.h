#pragma once
#include <Externals/stb/stb_truetype.h>

#include <Math/Vector/Vector2.h>

#include <Features/TextRenderer/AtlasData.h>

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <map>

class DXCommon;
class SRVManager;
class FontCache
{
public:
    static FontCache* GetInstance();

public:

    void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const Vector2& _windowSize);

    AtlasData* GetAtlasData(const std::string& _fontFilePath, float _fontSize);

private:
    ID3D12Device* device_; // D3D12デバイス
    ID3D12GraphicsCommandList* cmdList_; // コマンドリスト

    SRVManager* srvManager_; // SRVマネージャー
    DXCommon* dxCommon_; // DXCommon

    Vector2 windowSize_; // ウィンドウサイズ

    //                  fontpath    size    アトラス
    std::map<std::pair<std::string, float>, std::unique_ptr<AtlasData>> atlasDataMap_; // アトラスデータを格納するマップ

private:

    // singleton
    FontCache() = default;
    ~FontCache() = default;
    FontCache(const FontCache&) = delete;
    FontCache& operator=(const FontCache&) = delete;

};