#pragma once

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

class DXCommon;
class SRVManager
{
public:

    static SRVManager* GetInstance();
    void Initialize();

    void PreDraw();

    uint32_t Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSRVDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRVDescriptorHandle(uint32_t _index);

    void CreateSRVForTextrue2D(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format, UINT _mipLevel);
    void CreateSRVForStructureBuffer(uint32_t _index, ID3D12Resource* _resource, UINT _numElement,UINT _structureBufferStride);

    ID3D12DescriptorHeap* GetSRVHeap_() { return descriptorHeap_.Get(); }

private:
    static const uint32_t kMaxIndex_;
    uint32_t descriptorSize_;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

    uint32_t useIndex_ = 0;
    DXCommon* dxcommon_ = nullptr;



};