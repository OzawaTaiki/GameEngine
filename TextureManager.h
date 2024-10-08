#pragma once

#include <d3dx12.h>
#include <DirectXTex.h>

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <map>



class DXCommon;
class TextureManager
{
public:

    static TextureManager* GetInstance();

    void Initialize();
    void Update();

    void LoadTexture(const std::string& _filepath);
    void LoadTextureAtMaterial(const std::string& _filepath);

private:

    DirectX::ScratchImage GetMipImage(const std::string& _filepath);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& _metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* _texture , const DirectX::ScratchImage& _mipImages);

    static const std::string defaultDirpath_;

    DXCommon* dxCommon_ = nullptr;

    struct Texture
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandlerCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandlerGPU;
    };

    std::map<std::string, Texture> textures_ = {};

    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager operator=(const TextureManager&) = delete;

};