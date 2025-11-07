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
    void DrawRenderTexture(const std::string& _name);
    void DrawRenderTexture(uint32_t _index);

    void ClearAllRenderTarget();

    void SetRenderTexture(uint32_t _index);
    void SetRenderTexture(std::string _name);

    void SetDepthStencil(uint32_t _index);
    void SetDepthStencil(std::string _name);

    void SetSwapChainRenderTexture(IDXGISwapChain4* _swapChain);

    uint32_t CreateRenderTarget(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _colorFormat,const Vector4& _clearColor,bool _createDSV);

    RenderTarget* GetRenderTexture(std::string _name) { return renderTargets_[textureMap_[_name]].get(); }
    RenderTarget* GetRenderTexture(uint32_t _index) { return renderTargets_[_index].get(); }

    uint32_t CreateCubemapRenderTarget(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _colorFormat, const Vector4& _clearColor, bool _createDSV);

    void QueuePointLightShadowMapToSRV(const std::string& _name, uint32_t _index);
    void QueuePointLightShadowMapToSRV(uint32_t _handle, uint32_t _index);

    uint32_t CreateComputeOutputTexture(const std::string& _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    void TransitionForCompute(uint32_t _rtvIndex);
    void TransitionForRender(uint32_t _rtvIndex);

    void SetCubemapRenderTexture(uint32_t _handle);
    ID3D12Resource* GetCubemapResource(uint32_t _handle) const;

private:
    //TODO : DXCommonのもろもろをRTVManagerにおきかえ

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(uint32_t _width, uint32_t _height,uint32_t _rtvindex, DXGI_FORMAT _format, const Vector4& _clearColor);
    void CreateDepthStencilTextureResource(uint32_t _width, uint32_t _height, uint32_t _dsvindex);

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResourceWithUAV(uint32_t _width, uint32_t _height, uint32_t _rtvIndex, DXGI_FORMAT _format,
        const Vector4& _clearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f));


    Microsoft::WRL::ComPtr<ID3D12Resource> CreateCubemapResource(
        uint32_t _width,
        uint32_t _height,
        DXGI_FORMAT _format,
        const Vector4& _clearColor);

    uint32_t AllocateRTVIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTVDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPURTVDescriptorHandle(uint32_t _index);

    uint32_t AllocateDSVIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDSVDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDSVDescriptorHandle(uint32_t _index);

    static uint32_t winWidth_;
    static uint32_t winHeight_;
    static const uint32_t kMaxRTVIndex_;
    static const uint32_t kMaxDSVIndex_;
     uint32_t rtvDescriptorSize_;
     uint32_t dsvDescriptorSize_;


    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;

    uint32_t useIndexForRTV_ = 0;
    uint32_t useIndexForDSV_ = 0;
    DXCommon* dxcommon_ = nullptr;
    uint32_t backBufferCount_ = 0;

    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};

    float clearColor_[4] = { 0.4625f,0.925f,0.4625f,1.0f };

    std::map<std::string, uint32_t> textureMap_;
    std::map<uint32_t, std::unique_ptr<RenderTarget>> renderTargets_;
    std::map<uint32_t,Microsoft::WRL::ComPtr<ID3D12Resource>> dsvResources_;

    struct CubemapData {
        //std::vector<uint32_t> faceHandles;
        uint32_t srvIndex = 0;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    };
    std::map<uint32_t, CubemapData> cubemaps_;

    RTVManager() = default;
    ~RTVManager() = default;
    // コピー禁止
    RTVManager(const RTVManager&) = delete;
    RTVManager& operator=(const RTVManager&) = delete;


};
