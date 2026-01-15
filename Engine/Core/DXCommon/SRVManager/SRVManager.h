#pragma once

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>


namespace Engine {

class DXCommon;
class SRVManager
{
public:

    static SRVManager* GetInstance();
    void Initialize();

    void PreDraw();
    void PreDraw(ID3D12GraphicsCommandList* _commandList);

    uint32_t Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSRVDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRVDescriptorHandle(uint32_t _index);

    void CreateSRVForTexture2D(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format, UINT _mipLevel);
    void CreateSRVForStructureBuffer(uint32_t _index, ID3D12Resource* _resource, UINT _numElement,UINT _structureBufferStride);
    void CreateSRVForRenderTexture(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format);
    void CreateSRVForCubemap(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format);
    void CreateUAVForBuffer(uint32_t _index, ID3D12Resource* _resource, uint32_t _elementNum, size_t _elementSize);
    void CreateUAV(uint32_t _index, ID3D12Resource* _resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* _desc);

    void CreateUAVForTexture2D(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format);
    void SetComputeUAV(uint32_t _index, uint32_t _slot);
    void SetComputeSRV(uint32_t _index, uint32_t _slot);

    ID3D12DescriptorHeap* GetSRVHeap() { return descriptorHeap_.Get(); }

private:
    static const uint32_t kMaxIndex_;
    uint32_t descriptorSize_;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

    uint32_t useIndex_ = 0;
    DXCommon* dxcommon_ = nullptr;



};

} // namespace Engine
