#pragma once


#include <Core/DXCommon/RTV/RenderTexture.h>
#include <Math/Vector/Vector4.h>

#include <map>
#include <string>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class DXCommon;

class RTVManager
{
public:

    static RTVManager* GetInstance();

    void Initialize(size_t _backBufferCount, uint32_t _winWidth, uint32_t _winHeight);


    void SetRenderTexture(uint32_t _index, uint32_t _dsvHandle);
    void SetRenderTexture(std::string _name, uint32_t _dsvHandle);

    void SetSwapChainRenderTexture(IDXGISwapChain4* _swapChain, uint32_t _dsvHandle);

    uint32_t CreateRenderTexture(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor);

    RenderTexture* GetRenderTexture(std::string _name) { return renderTextures_[textureMap_[_name]].get(); }
    RenderTexture* GetRenderTexture(uint32_t _index) { return renderTextures_[_index].get(); }



private:
    //TODO : DXCommonのもろもろをRTVManagerにおきかえ

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor);

    uint32_t Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTVDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPURTVDescriptorHandle(uint32_t _index);

    static uint32_t winWidth_;
    static uint32_t winHeight_;
    static const uint32_t kMaxIndex_;
    uint32_t descriptorSize_;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

    uint32_t useIndex_ = 0;
    DXCommon* dxcommon_ = nullptr;
    uint32_t backBufferCount_ = 0;

    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};

    float clearColor_[4] = { 0.4625f,0.925f,0.4625f,1.0f };

    std::map<std::string, uint32_t> textureMap_;
    std::map<std::uint32_t, std::unique_ptr<RenderTexture>> renderTextures_;

    RTVManager() = default;
    ~RTVManager() = default;
    // コピー禁止
    RTVManager(const RTVManager&) = delete;
    RTVManager& operator=(const RTVManager&) = delete;


};
