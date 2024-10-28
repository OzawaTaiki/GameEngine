#pragma once
#include "Vector2.h"

#include <d3dx12.h>
#include <DirectXTex.h>

#include <wrl.h>
#include <string>
#include <map>
#include <optional>



class DXCommon;
class TextureManager
{
public:

    static TextureManager* GetInstance();

    void Initialize();
    void Update();

    uint32_t Load(const std::string& _filepath, const std::string& defaultDirpath_ = "Resources/images/");
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t _textureHandle);
    Vector2 GetTextureSize(uint32_t _textureHandle);

private:

    uint32_t LoadTexture(const std::string& _filepath);
    std::optional<uint32_t> IsTextureLoaded(const std::string& _filepath);


    DirectX::ScratchImage GetMipImage(const std::string& _filepath);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& _metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* _texture , const DirectX::ScratchImage& _mipImages);

    DXCommon* dxCommon_ = nullptr;

    struct Texture
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandlerCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandlerGPU;
    };

    std::unordered_map<std::string, uint32_t> keys_ = {};
    std::map<uint32_t, Texture> textures_ = {};

    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager operator=(const TextureManager&) = delete;

};