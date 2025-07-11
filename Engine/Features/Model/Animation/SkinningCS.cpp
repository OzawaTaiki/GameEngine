#include "SkinningCS.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

#include <Debug/Debug.h>

#include <dxcapi.h>


SkinningCS::SkinningCS(ID3D12PipelineState* _computePipeline, ID3D12RootSignature* _rootSignature)
    : computePipeline_(_computePipeline), rootSignature_(_rootSignature)
{
    dxCommon_ = DXCommon::GetInstance();
    commandList_ = dxCommon_->GetCommandList();
    srvManager_ = SRVManager::GetInstance();
}

SkinningCS::~SkinningCS()
{
    if(computePipeline_)
        computePipeline_ = nullptr;

    if (rootSignature_)
        rootSignature_ = nullptr;

}

void SkinningCS::Execute()
{
    srvManager_->PreDraw();

    commandList_->SetPipelineState(computePipeline_);
    commandList_->SetComputeRootSignature(rootSignature_);
    commandList_->SetComputeRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(matrixPaletteSrvIndex_));
    commandList_->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(inputVertexSrvIndex_));
    commandList_->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(influenceSrvIndex_));
    commandList_->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(outputVertexUavIndex_));
    commandList_->SetComputeRootConstantBufferView(4, skinnedInformationResources_->GetGPUVirtualAddress());
    commandList_->Dispatch(UINT(*vertexNum_ + 1023)/1024, 1, 1);
}

uint32_t SkinningCS::CreateSRVForInputVertexResource(ID3D12Resource* _resource, uint32_t _vertexNum)
{
    CreateInfoResource(_vertexNum);

    inputVertexSrvIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructureBuffer(inputVertexSrvIndex_, _resource, _vertexNum, sizeof(VertexData));

    return inputVertexSrvIndex_;
}

uint32_t SkinningCS::CreateSRVForOutputVertexResource(ID3D12Resource* _resource, uint32_t _vertexNum)
{
    outputVertexUavIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForUAV(outputVertexUavIndex_, _resource, _vertexNum, sizeof(VertexData));
    return outputVertexUavIndex_;

}

uint32_t SkinningCS::CreateSRVForInfluenceResource(ID3D12Resource* _resource, uint32_t _vertexNum)
{
    influenceSrvIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructureBuffer(influenceSrvIndex_, _resource, _vertexNum, sizeof(VertexInfluenceData));
    return influenceSrvIndex_;
}

uint32_t SkinningCS::CreateSRVForMatrixPaletteResource(ID3D12Resource* _resource, uint32_t _jointNum)
{
    matrixPaletteSrvIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructureBuffer(matrixPaletteSrvIndex_, _resource, _jointNum, sizeof(WellForGPU));
    return matrixPaletteSrvIndex_;
}

Microsoft::WRL::ComPtr<ID3D12Resource> SkinningCS::CreateOutputVertexResource(size_t _vertexSize)
{
    DXCommon* dxCommon = DXCommon::GetInstance();

    Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource = nullptr;

    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;


    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeof(VertexData) * _vertexSize;
    //バッファの場合はこれらを１にする決まり
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;


    //実際に頂点リソースを作る
    HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
        IID_PPV_ARGS(&outputVertexResource));

    assert(SUCCEEDED(hr));

    return outputVertexResource;
}

void SkinningCS::CreateInfoResource(uint32_t _vertexNum)
{
    skinnedInformationResources_ = dxCommon_->CreateBufferResource(sizeof(uint32_t));
    skinnedInformationResources_->Map(0, nullptr, reinterpret_cast<void**>(&vertexNum_));
    *vertexNum_ = _vertexNum;
}
