#include "BoxFilter.h"

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>

#include <cassert>
#include <dxcapi.h>

void BoxFilter::Initialize()
{
    CreateConstantBuffer(sizeof(BoxFilterData));

    // PSOとルートシグネチャを生成
    CreateRootSignature();
    CreatePipelineState();
}

void BoxFilter::Apply(const std::string& input, const std::string& output)
{
    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    // 定数バッファを更新
    if(postEffectData_)
        UpdateConstantBuffer(postEffectData_, sizeof(BoxFilterData));

    // パイプラインステート設定
    cmdList->SetPipelineState(pipelineState_.Get());
    cmdList->SetComputeRootSignature(rootSignature_.Get());

    // ルートパラメータ設定
    // CBV (b0) - 定数バッファ
    cmdList->SetComputeRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

    auto rtvManager = RTVManager::GetInstance();
    UINT inputSRVIndex = rtvManager->GetRenderTexture(input)->GetSRVindexofRTV();
    UINT outputUAVIndex = rtvManager->GetRenderTexture(output)->GetSRVindexofRTV();

    rtvManager->GetRenderTexture(input)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rtvManager->GetRenderTexture(output)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // SRV (t0) - 入力テクスチャ
    cmdList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(inputSRVIndex));
    // UAV (u0) - 出力テクスチャ
    cmdList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(outputUAVIndex));


    // スレッドグループ数計算 (8x8のスレッドグループ)
    uint32_t dispatchX = (WinApp::kWindowWidth_ + 7) / 8;
    uint32_t dispatchY = (WinApp::kWindowHeight_ + 7) / 8;

    // Dispatch実行
    cmdList->Dispatch(dispatchX, dispatchY, 1);


    rtvManager->GetRenderTexture(output)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void BoxFilter::SetData(const PostEffectBaseData* data)
{
    if (data)
    {
        const BoxFilterData* boxFilterData = dynamic_cast<const BoxFilterData*>(data);
        if (boxFilterData)
        {
            UpdateConstantBuffer(boxFilterData, sizeof(BoxFilterData));
        }
        else
        {
            // エラーハンドリング: データの型が不正な場合
            assert(false && "Invalid BoxFilterData type");
        }
    }
    else
    {
        // エラーハンドリング: データがnullptrの場合
        assert(false && "BoxFilterData is nullptr");
    }
}

void BoxFilter::CreateRootSignature()
{
    HRESULT hr = S_FALSE;

#pragma region RootSignature
    // RootSignatureを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute用

    // descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};

    // SRV (t0) - 入力テクスチャ
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // UAV (u0) - 出力テクスチャ
    descriptorRange[1].BaseShaderRegister = 0;
    descriptorRange[1].NumDescriptors = 2;
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // CBV (b0) - 定数バッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // SRV テーブル (t0)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // UAV テーブル (u0)
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    // Compute用なのでサンプラーは不要
    descriptionRootSignature.pStaticSamplers = nullptr;
    descriptionRootSignature.NumStaticSamplers = 0;

    // シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
#pragma endregion
}

void BoxFilter::CreatePipelineState()
{
    HRESULT hr = S_FALSE;

#pragma region Shader
    // Compute Shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob =
        PSOManager::GetInstance()->ComplieShader(L"BoxFilter.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);
#pragma endregion

    // Compute PSOを生成する
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
    computePipelineStateDesc.pRootSignature = rootSignature_.Get();
    computePipelineStateDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };
    computePipelineStateDesc.NodeMask = 0;
    computePipelineStateDesc.CachedPSO.pCachedBlob = nullptr;
    computePipelineStateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
    computePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSOを生成
    hr = DXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));

}
