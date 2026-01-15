#include "SilhouetteDetection.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/Debug.h>
#include <d3dx12.h>


namespace Engine {

Microsoft::WRL::ComPtr<ID3D12PipelineState> SilhouetteDetection::edgeDetectionPSO_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12PipelineState> SilhouetteDetection::silhouetteEdgePSO_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12RootSignature> SilhouetteDetection::rootSignature_ = nullptr;
bool SilhouetteDetection::isCreated_ = false;

const uint32_t SilhouetteDetection::kDefaultEdgeInfoCount_ = 10000;
const uint32_t SilhouetteDetection::kEdgeInfoCountOffset_ = 100;
const uint32_t SilhouetteDetection::kDefaultSilhouetteEdgeCount_ = 256;
const uint32_t SilhouetteDetection::kDedaultPairEdgeCount_ = 256;

SilhouetteDetection::~SilhouetteDetection()
{
    if (edgeDetectionPSO_)
        edgeDetectionPSO_.Reset();

    if (silhouetteEdgePSO_)
        silhouetteEdgePSO_.Reset();

    if (rootSignature_)
        rootSignature_.Reset();

    isCreated_ = false;
}

void SilhouetteDetection::Initialize(uint32_t _inputVertexSrvIndex, uint32_t _inputIndexSrvIndex, uint32_t _vertexCount, uint32_t _indexCount, const Vector3& _lightDirection)
{
    dxCommon_ = DXCommon::GetInstance();

    inputVertexSrvIndex_ = _inputVertexSrvIndex;
    inputIndexSrvIndex_ = _inputIndexSrvIndex;

    if (!isCreated_)
        CreatePipeline();

    CreateConstantBuffer();

    constantBufferData_->lightDirection = _lightDirection;
    constantBufferData_->vertexCount = _vertexCount;
    constantBufferData_->indexCount = _indexCount;
    constantBufferData_->debug_frameCount = 0;

    CreateResources();
}

void SilhouetteDetection::Initialize(uint32_t _inputVertexSrvIndex, uint32_t _inputIndexSrvIndex, uint32_t _vertexCount, uint32_t _indexCount, Vector3* _lightDirection)
{
    dxCommon_ = DXCommon::GetInstance();

    if (_lightDirection)
        lightDirection_ = _lightDirection;

    inputVertexSrvIndex_ = _inputVertexSrvIndex;
    inputIndexSrvIndex_ = _inputIndexSrvIndex;

    if (!isCreated_)
        CreatePipeline();

    CreateConstantBuffer();

    constantBufferData_->lightDirection = *lightDirection_;
    constantBufferData_->vertexCount = _vertexCount;
    constantBufferData_->indexCount = _indexCount;
    constantBufferData_->debug_frameCount = 0;

    CreateResources();

}

uint32_t SilhouetteDetection::CreateSRVForInputIndexResource(ID3D12Resource* _resource, uint32_t _indexCount)
{
    SRVManager* srvManager = SRVManager::GetInstance();

    uint32_t index = srvManager->Allocate();
    srvManager->CreateSRVForStructureBuffer(index, _resource, _indexCount, sizeof(uint32_t));

    return index;
}

void SilhouetteDetection::Execute()
{
    if (lightDirection_)
        constantBufferData_->lightDirection = *lightDirection_;

    constantBufferData_->debug_frameCount++;

    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    SRVManager* srvManager = SRVManager::GetInstance();

    srvManager->PreDraw();

    commandList->SetPipelineState(edgeDetectionPSO_.Get());
    commandList->SetComputeRootSignature(rootSignature_.Get());

    commandList->SetComputeRootDescriptorTable(0, srvManager->GetGPUSRVDescriptorHandle(inputVertexSrvIndex_));
    commandList->SetComputeRootDescriptorTable(1, srvManager->GetGPUSRVDescriptorHandle(inputIndexSrvIndex_));
    commandList->SetComputeRootDescriptorTable(2, srvManager->GetGPUSRVDescriptorHandle(edgeInfoSrvIndex_));
    commandList->SetComputeRootDescriptorTable(3, srvManager->GetGPUSRVDescriptorHandle(counterSrvIndex_));
    commandList->SetComputeRootDescriptorTable(4, srvManager->GetGPUSRVDescriptorHandle(silhouetteEdgeUavIndex_));
    commandList->SetComputeRootDescriptorTable(5, srvManager->GetGPUSRVDescriptorHandle(pairEdgeSrvIndex_));
    commandList->SetComputeRootConstantBufferView(6, constantBuffer_->GetGPUVirtualAddress());

    uint32_t triangleCount = constantBufferData_->indexCount / 3;
    uint32_t threadGroupsX = (triangleCount + 255) / 256;
    commandList->Dispatch(threadGroupsX, 1, 1);

    // シルエットエッジの数を取得
    commandList->SetPipelineState(silhouetteEdgePSO_.Get());

    commandList->SetComputeRootDescriptorTable(0, srvManager->GetGPUSRVDescriptorHandle(inputVertexSrvIndex_));
    commandList->SetComputeRootDescriptorTable(1, srvManager->GetGPUSRVDescriptorHandle(inputIndexSrvIndex_));
    commandList->SetComputeRootDescriptorTable(2, srvManager->GetGPUSRVDescriptorHandle(edgeInfoSrvIndex_));
    commandList->SetComputeRootDescriptorTable(3, srvManager->GetGPUSRVDescriptorHandle(counterSrvIndex_));
    commandList->SetComputeRootDescriptorTable(4, srvManager->GetGPUSRVDescriptorHandle(silhouetteEdgeUavIndex_));
    commandList->SetComputeRootDescriptorTable(5, srvManager->GetGPUSRVDescriptorHandle(pairEdgeSrvIndex_));
    commandList->SetComputeRootConstantBufferView(6, constantBuffer_->GetGPUVirtualAddress());

    uint32_t maxEdgeCount = triangleCount * 3;
    threadGroupsX = (maxEdgeCount + 255) / 256;

    commandList->Dispatch(threadGroupsX, 1, 1);




}

void SilhouetteDetection::CreatePipeline()
{
    isCreated_ = true;

    HRESULT hr = S_FALSE;

    PSOManager* psoManager = PSOManager::GetInstance();

    // ルートシグネチャの生成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[6] = {};
    // inputVertex
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // inputIndex
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // edgeInfo
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 0;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // counter
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 1;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // silhouetteEdge
    descriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[4].NumDescriptors = 1;
    descriptorRanges[4].BaseShaderRegister = 2;
    descriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // pairEdge
    descriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[5].NumDescriptors = 1;
    descriptorRanges[5].BaseShaderRegister = 3;
    descriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

#pragma region RootParameter

    D3D12_ROOT_PARAMETER rootParameters[7] = {};

    // inputVertex
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // inputIndex
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // edgeInfo
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // counter
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[3];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // silhouetteEdge
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[4].DescriptorTable.pDescriptorRanges = &descriptorRanges[4];
    rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;

    // pairEdge
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].DescriptorTable.pDescriptorRanges = &descriptorRanges[5];
    rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;

    // silhouetteEdge
    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[6].Descriptor.ShaderRegister = 0;




    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

#pragma endregion


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


    // ComputeShaderのコンパイル
    // シェーダーの読み込み
    Microsoft::WRL::ComPtr<IDxcBlob> edgeDetectionBlob = psoManager->ComplieShader(L"SilhouetteDetection.CS.hlsl", L"cs_6_0", L"CS_CreateEdgeInfo");


    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { edgeDetectionBlob->GetBufferPointer(), edgeDetectionBlob->GetBufferSize() };

    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&edgeDetectionPSO_));
    assert(SUCCEEDED(hr));

    // シェーダーの読み込み
    Microsoft::WRL::ComPtr<IDxcBlob> silhouetteEdgeBlob = psoManager->ComplieShader(L"SilhouetteDetection.CS.hlsl", L"cs_6_0", L"CS_DetectSilhouette");

    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { silhouetteEdgeBlob->GetBufferPointer(), silhouetteEdgeBlob->GetBufferSize() };

    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&silhouetteEdgePSO_));
    assert(SUCCEEDED(hr));

}

void SilhouetteDetection::CreateConstantBuffer()
{
    // constantBufferの生成
    constantBuffer_ = dxCommon_->CreateBufferResource(sizeof(ConstantBufferData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constantBufferData_));
}

void SilhouetteDetection::CreateResources()
{
    SRVManager* srvManager = SRVManager::GetInstance();

    // edgeInfoの生成
    const uint32_t triangleCount = constantBufferData_->indexCount / 3;
    const uint32_t edgeInfoCount = triangleCount * 3 + kEdgeInfoCountOffset_;
    edgeInfoResource_ = dxCommon_->CreateUAVBufferResource(sizeof(EdgeInfoData) * edgeInfoCount);

    edgeInfoSrvIndex_ = srvManager->Allocate();
    srvManager->CreateUAVForBuffer(edgeInfoSrvIndex_, edgeInfoResource_.Get(), edgeInfoCount, sizeof(EdgeInfoData));


    const uint32_t counterCount = 4;

    // counterの生成
    counterResource_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * counterCount);

    counterSrvIndex_ = srvManager->Allocate();
    srvManager->CreateUAVForBuffer(counterSrvIndex_, counterResource_.Get(), counterCount, sizeof(uint32_t));


    // silhouetteEdgeの生成
    silhouetteEdgeResource_ = dxCommon_->CreateUAVBufferResource(sizeof(SilhouetteEdgeData) * kDefaultSilhouetteEdgeCount_);

    silhouetteEdgeUavIndex_ = srvManager->Allocate();
    srvManager->CreateUAVForBuffer(silhouetteEdgeUavIndex_, silhouetteEdgeResource_.Get(), kDefaultSilhouetteEdgeCount_, sizeof(SilhouetteEdgeData));


    // pairEdgeの生成
    pairEdgeResource_ = dxCommon_->CreateUAVBufferResource(sizeof(PairEdgeData) * kDefaultSilhouetteEdgeCount_ );

    pairEdgeSrvIndex_ = srvManager->Allocate();
    srvManager->CreateUAVForBuffer(pairEdgeSrvIndex_, pairEdgeResource_.Get(), kDefaultSilhouetteEdgeCount_, sizeof(PairEdgeData));


    std::vector<EdgeInfoData> initialEdgeInfo(edgeInfoCount);
    for (auto& edge : initialEdgeInfo) {
        edge.triangleA = 0xFFFFFFFF;
        edge.triangleB = 0xFFFFFFFF;
        edge.isBoundary = false;
        edge.isProcessed = false;
    }

    // リソース作成時に初期データを渡す
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        sizeof(EdgeInfoData) * edgeInfoCount,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    D3D12_SUBRESOURCE_DATA initialData = {};
    initialData.pData = initialEdgeInfo.data();
    initialData.RowPitch = sizeof(EdgeInfoData) * edgeInfoCount;
    initialData.SlicePitch = initialData.RowPitch;



    UINT64 uploadBufferSize = GetRequiredIntermediateSize(edgeInfoResource_.Get(), 0, 1);

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    dxCommon_->GetDevice()->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer_)
    );

    // 初期データのアップロード
    UpdateSubresources(dxCommon_->GetCommandList(), edgeInfoResource_.Get(), uploadBuffer_.Get(), 0, 0, 1, &initialData);




}

} // namespace Engine
