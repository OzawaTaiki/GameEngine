#include "SRVManager.h"

#include "DXCommon.h"

const uint32_t SRVManager::kMAxIndex_ = 4096;

void SRVManager::Initialize()
{
    dxcommon_ = DXCommon::GetInstance();

    descriptorHeap_ = dxcommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMAxIndex_,true);
    descriptorSize_ = dxcommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SRVManager::Allocate()
{
    uint32_t index = useIndex_++;
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