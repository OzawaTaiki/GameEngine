#include "SRVManager.h"

#include "DXCommon.h"

const uint32_t SRVManager::kMaxIndex_ = 1024;

void SRVManager::Initialize()
{
    dxcommon_ = DXCommon::GetInstance();

    descriptorHeap_ = dxcommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxIndex_,true);
    descriptorSize_ = dxcommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void SRVManager::PreDraw()
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
    dxcommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

uint32_t SRVManager::Allocate()
{
    uint32_t index = useIndex_++;
    if (useIndex_ >= kMaxIndex_)
        throw std::runtime_error("over MaxSRVindex");
    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUSRVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUSRVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize_ * _index);
    return handleGPU;
}

void SRVManager::CreateSRVForTextrue2D(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format, UINT _mipLevel)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = _format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = _mipLevel;

    dxcommon_->GetDevice()->CreateShaderResourceView(_resource, &srvDesc, GetCPUSRVDescriptorHandle(_index));
}

void SRVManager::CreateSRVForStructureBuffer(uint32_t _index, ID3D12Resource* _resource, UINT _numElement, UINT _structureBufferStride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = _numElement;
    srvDesc.Buffer.StructureByteStride = _structureBufferStride;

    dxcommon_->GetDevice()->CreateShaderResourceView(_resource, &srvDesc, GetCPUSRVDescriptorHandle(_index));

}
