#include <Core/DXCommon/SRVManager/SRVManager.h>

#include <Core/DXCommon/DXCommon.h>

const uint32_t SRVManager::kMaxIndex_ = 1024;

SRVManager* SRVManager::GetInstance()
{
    static SRVManager instance;
    return &instance;
}

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

void SRVManager::PreDraw(ID3D12GraphicsCommandList* _commandList)
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
    _commandList->SetDescriptorHeaps(1, descriptorHeaps);
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

void SRVManager::CreateSRVForRenderTexture(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = _format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    dxcommon_->GetDevice()->CreateShaderResourceView(_resource, &srvDesc, GetCPUSRVDescriptorHandle(_index));
}

void SRVManager::CreateSRVForCubemap(uint32_t _index, ID3D12Resource* _resource, DXGI_FORMAT _format)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = _format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;

    dxcommon_->GetDevice()->CreateShaderResourceView(_resource, &srvDesc, GetCPUSRVDescriptorHandle(_index));
}

void SRVManager::CreateSRVForUAV(uint32_t _index, ID3D12Resource* _resource, uint32_t _elementNum, size_t _elementSize)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = _elementNum;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.StructureByteStride = UINT(_elementSize);

    dxcommon_->GetDevice()->CreateUnorderedAccessView(_resource, nullptr, &uavDesc, GetCPUSRVDescriptorHandle(_index));
}

void SRVManager::CreateUAV(uint32_t _index, ID3D12Resource* _resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* _desc)
{
    // UAVを作成して指定されたインデックスのディスクリプタヒープに設定
    dxcommon_->GetDevice()->CreateUnorderedAccessView(
        _resource,       // リソース
        nullptr,         // カウンターリソース
        _desc,           // UAV記述子
        GetCPUSRVDescriptorHandle(_index) // ディスクリプタハンドル
    );
}