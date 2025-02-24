#include "SkinningCS.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

#include <Debug/Debug.h>

#include <dxcapi.h>


Microsoft::WRL::ComPtr<ID3D12PipelineState> SkinningCS::computePipeline_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12RootSignature> SkinningCS::rootSignature_ = nullptr;

bool SkinningCS::isCreated_ = false;



SkinningCS::SkinningCS()
{
    dxCommon_ = DXCommon::GetInstance();
    commandList_ = dxCommon_->GetCommandList();
    srvManager_ = SRVManager::GetInstance();


    if (!isCreated_)
    {
        CreateComputePipeline();
        isCreated_ = true;
    }
}

SkinningCS::~SkinningCS()
{
    if (computePipeline_)
        computePipeline_.Reset();

    if (rootSignature_)
        rootSignature_.Reset();

}

void SkinningCS::Initialize()
{
    //CreateInputVertexResource( _vertexSize);
}

void SkinningCS::Execute()
{
    srvManager_->PreDraw();

    commandList_->SetPipelineState(computePipeline_.Get());
    commandList_->SetComputeRootSignature(rootSignature_.Get());
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

//void SkinningCS::CreateInfluenceResource( uint32_t _influenceSize)
//{
//    influenceResource_ = dxCommon_->CreateBufferResource(sizeof(VertexInfluenceData) * _influenceSize);
//
//    influenceSrvIndex_ = srvManager_->Allocate();
//    srvManager_->CreateSRVForStructureBuffer(influenceSrvIndex_, influenceResource_.Get(), _influenceSize, sizeof(VertexInfluenceData));
//
//    influenceResource_->Map(0, nullptr, reinterpret_cast<void**>(&influence_));
//
//}
//
//void SkinningCS::CreateMatrixPaletteResource( uint32_t _influenceSize)
//{
//    matrixPaletteResource_ = dxCommon_->CreateBufferResource(sizeof(WellForGPU) * _influenceSize);
//
//    matrixPaletteSrvIndex_ = srvManager_->Allocate();
//    srvManager_->CreateSRVForStructureBuffer(matrixPaletteSrvIndex_, matrixPaletteResource_.Get(), _influenceSize, sizeof(WellForGPU));
//
//    matrixPaletteResource_->Map(0, nullptr, reinterpret_cast<void**>(&matrixPalette_));
//}
//
//void SkinningCS::CreateVertexNumResource(uint32_t _vertexNum)
//{
//    skinnedInformationResources_ = dxCommon_->CreateBufferResource(sizeof(uint32_t));
//
//    skinnedInformationResources_->Map(0, nullptr, reinterpret_cast<void**>(&_vertexNum));
//}

void SkinningCS::CreateInfoResource(uint32_t _vertexNum)
{
    skinnedInformationResources_ = dxCommon_->CreateBufferResource(sizeof(uint32_t));
    skinnedInformationResources_->Map(0, nullptr, reinterpret_cast<void**>(&vertexNum_));
    *vertexNum_ = _vertexNum;
}

void SkinningCS::CreateComputePipeline()
{
    isCreated_ = true;
    HRESULT hr = S_FALSE;


    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[4] = {};
    // gMatrixPalette
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // inputVertex
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // gInfluence
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 2;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // outputVertex
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 0;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER rootParameters[5] = {};

    // gMatrixPalette
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // inputVertex
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // gInfluence
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // outputVertex
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[3];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // skinnedInformation
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[4].Descriptor.ShaderRegister = 0;


    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);


    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));


    // ComputeShaderの設定
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = nullptr;
    // ComputeShaderのコンパイル
    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Skinning.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipeline_));
    assert(SUCCEEDED(hr));


}
