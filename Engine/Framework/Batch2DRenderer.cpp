#include "Batch2DRenderer.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/Debug.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Framework/LayerSystem/LayerSystem.h>

Batch2DRenderer* Batch2DRenderer::GetInstance()
{
    static Batch2DRenderer instance;
    return &instance;
}

void Batch2DRenderer::Initialize()
{
    CreateRootSignature();
    CreatePipelineStateObject();

    CreateVertexBuffer();
    CreateInstanceDataSRV();

    // 初期容量確保
    drawDataList_.reserve(kMaxInstanceCount_);
    sortIndices_.reserve(kMaxInstanceCount_);
}

void Batch2DRenderer::Render()
{
    if (drawDataList_.empty())
        return;

    SortData();
    UploadData();
    BuildDrawCommands();


    // 描画コマンド発行
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetGraphicsRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(instanceDataSRVIndex_));
    commandList->SetGraphicsRootConstantBufferView(1, instanceResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(0));

    for (const auto& cmd : drawCommands_)
    {
        LayerSystem::SetLayer(cmd.layer);
        commandList->DrawInstanced(6, cmd.instanceCount, 0, cmd.startInstance);
    }

    Reset();
}

void Batch2DRenderer::AddInstace(const InstanceData& _instance, const std::vector<VertexData>& _v)
{
    DrawData data;
    data.instance = _instance;
    data.layer = LayerSystem::GetCurrentLayerID();
    data.order = static_cast<uint32_t>(drawDataList_.size());
    data.vertices = _v;

    drawDataList_.push_back(data);
    sortIndices_.push_back(static_cast<uint32_t>(sortIndices_.size()));

}

void Batch2DRenderer::CreateVertexBuffer()
{
    vertexResource_ = DXCommon::GetInstance()->
        CreateBufferResource(sizeof(VertexData) * 6 * kMaxInstanceCount_);// 6頂点 * インスタンス数
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexMap_));

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6 * kMaxInstanceCount_;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

}

void Batch2DRenderer::CreateInstanceDataSRV()
{
    instanceResource_ = DXCommon::GetInstance()->
        CreateBufferResource(sizeof(InstanceData) * kMaxInstanceCount_);
    instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&instanceMap_));

    instanceDataSRVIndex_ = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->
        CreateSRVForStructureBuffer(instanceDataSRVIndex_, instanceResource_.Get(), kMaxInstanceCount_, sizeof(InstanceData));

    //ZeroMemory(&instanceMap_[0], sizeof(InstanceData) * kMaxInstanceCount_);
}

void Batch2DRenderer::Reset()
{
    drawDataList_.clear();
    sortIndices_.clear();
}

void Batch2DRenderer::CreatePipelineStateObject()
{
    HRESULT hr = S_FALSE;
    PSOManager* psoManager = PSOManager::GetInstance();

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = false;
#pragma endregion

#pragma region InputLayout
    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "COLOR";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
        psoManager->ComplieShader(L"Batch2DInstancing.hlsl", L"vs_6_0", L"VSMain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob =
        psoManager->ComplieShader(L"Batch2DInstancing.hlsl", L"ps_6_0",L"PSMain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = psoManager->GetBlendDesc(PSOFlags::BlendMode::Normal);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = psoManager->GetRasterizerDesc(PSOFlags::CullMode::None);

#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();                                         // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    hr = DXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

void Batch2DRenderer::CreateRootSignature()
{
    // sampler
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
    // テクスチャ用
    descriptorRange[0].BaseShaderRegister = 1;
    descriptorRange[0].NumDescriptors = UINT_MAX;//数は最大
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // インスタンスデータ用
    descriptorRange[1].BaseShaderRegister = 0;//
    descriptorRange[1].NumDescriptors = 1;//数は最大
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // instanceData
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // ortho
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0; // b0

    // テクスチャ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;


    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
                                                     signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void Batch2DRenderer::SortData()
{
    std::stable_sort(sortIndices_.begin(), sortIndices_.end(), [&](uint32_t a, uint32_t b)
                     {
                         const auto& dataA = drawDataList_[a];
                         const auto& dataB = drawDataList_[b];

                         // layerで昇順、同じlayerならorderで昇順
                         if (dataA.layer != dataB.layer)
                             return dataA.layer < dataB.layer;

                         return dataA.order < dataB.order;
                     });
}

void Batch2DRenderer::UploadData()
{
    for (size_t i = 0; i < sortIndices_.size(); ++i)
    {
        uint32_t index = sortIndices_[i];  // ソート後のインデックス
        const DrawData& data = drawDataList_[index];


        // 頂点データをコピー
        const size_t vertexCount = 6; // 2D四角形は6頂点
        std::copy_n(data.vertices.data(), vertexCount, &vertexMap_[i * vertexCount]);
        // インスタンスデータをコピー
        instanceMap_[i] = data.instance;
    }
}

void Batch2DRenderer::BuildDrawCommands()
{
    drawCommands_.clear();

    if (sortIndices_.empty()) return;

    // レイヤーごとにグループ化
    DrawCommand currentCommand;
    currentCommand.layer = drawDataList_[sortIndices_[0]].layer;
    currentCommand.startInstance = 0;
    currentCommand.instanceCount = 1;

    for (size_t i = 1; i < sortIndices_.size(); ++i)
    {
        int currentLayer = drawDataList_[sortIndices_[i]].layer;

        if (currentLayer == currentCommand.layer)
        {
            // 同じレイヤー：カウントを増やす
            currentCommand.instanceCount++;
        }
        else
        {
            // レイヤーが変わった：現在のコマンドを保存
            drawCommands_.push_back(currentCommand);

            // 新しいコマンドを開始
            currentCommand.layer = currentLayer;
            currentCommand.startInstance = static_cast<uint32_t>(i);
            currentCommand.instanceCount = 1;
        }
    }
    drawCommands_.push_back(currentCommand);
}
