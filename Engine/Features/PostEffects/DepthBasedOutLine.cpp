#include "DepthBasedOutline.h"

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Framework/LayerSystem/LayerSystem.h>
#include <Debug/Debug.h>
#include <Features/Camera/Camera/Camera.h>

#include <Math/Matrix/MatrixFunction.h>

#include <Debug/ImGuiDebugManager.h>

void DepthBasedOutLine::Initialize()
{
    CreateConstantBuffer(sizeof(DepthBasedOutLineData));

    // PSOとルートシグネチャを生成
    CreateRootSignature();
    CreatePipelineState();
}

void DepthBasedOutLine::Apply(const std::string& _input, const std::string& _output)
{
    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    UpdateData();

    // パイプラインステート設定
    cmdList->SetPipelineState(pipelineState_.Get());
    cmdList->SetComputeRootSignature(rootSignature_.Get());

    // ルートパラメータ設定
    // CBV (b0) - 定数バッファ
    cmdList->SetComputeRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

    auto rtvManager = RTVManager::GetInstance();
    UINT inputSRVIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofRTV();
    uint32_t outputUAVIndex = LayerSystem::GetUAVIndex(_output);
    uint32_t depthSRVIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofDSV();

    rtvManager->GetRenderTexture(_input)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rtvManager->GetRenderTexture(_output)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // SRV (t0) - 入力テクスチャ
    cmdList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(inputSRVIndex));
    // SRV (t1) 深度テクスチャ
    cmdList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(depthSRVIndex));
    // UAV (u0) - 出力テクスチャ
    cmdList->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(outputUAVIndex));

    // スレッドグループ数計算 (8x8のスレッドグループ)
    uint32_t dispatchX = (WinApp::kWindowWidth_ + 7) / 8;
    uint32_t dispatchY = (WinApp::kWindowHeight_ + 7) / 8;

    // Dispatch実行
    cmdList->Dispatch(dispatchX, dispatchY, 1);


    rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    rtvManager->GetRenderTexture(_output)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void DepthBasedOutLine::SetData(DepthBasedOutLineData* _data)
{
    if (_data)
    {
        data_ = _data;
    }
}

void DepthBasedOutLine::SetCamera(Camera* camera)
{
    if (camera)
    {
        camera_ = camera;
    }
}

void DepthBasedOutLine::CreatePipelineState()
{

    HRESULT hr = S_FALSE;

#pragma region Shader
    // Compute Shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob =
        PSOManager::GetInstance()->ComplieShader(L"DepthBasedOutline.CS.hlsl", L"cs_6_0");
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

void DepthBasedOutLine::CreateRootSignature()
{
    HRESULT hr = S_OK;


    // descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[3] = {};

    // SRV (t0) - 入力テクスチャ
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // SRV (t1) - depthテクスチャ
    descriptorRange[1].BaseShaderRegister = 1;
    descriptorRange[1].NumDescriptors = 1;
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // UAV (u0) - 出力テクスチャ
    descriptorRange[2].BaseShaderRegister = 0;
    descriptorRange[2].NumDescriptors = 1;
    descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ディスクリプタテーブルの作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // コンスタントバッファ (b0) DepthBasedOutlineData
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // テクスチャ用ディスクリプタテーブル
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];

    // 深度テクスチャ用ディスクリプタテーブル
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];

    // 出力テクスチャ用ディスクリプタテーブル
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRange[2];


    // ルートシグネチャ記述の設定
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute用

    // ルートシグネチャのシリアライズと作成
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(
        &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false && "Failed to serialize root signature");
        return;
    }

    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));

    if (FAILED(hr)) {
        assert(false && "Failed to create root signature");
        return;
    }
}

void DepthBasedOutLine::UpdateData()
{
    if (!data_ || !camera_)
        return;

    // カメラの逆射影行列を設定
    data_->inverseViewProjectionMatrix = Inverse(camera_->GetViewProjection());
    // 定数バッファを更新
    UpdateConstantBuffer(data_, sizeof(DepthBasedOutLineData));
}

void DepthBasedOutLineData::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    ImGui::SliderFloat("Edge Threshold", &edgeThreshold, 0.0f, 10.0f);
    ImGui::SliderFloat("Edge Intensity", &edgeIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Edge Width", &edgeWidth, 1.0f, 3.0f);
    ImGui::ColorEdit3("Edge Color", &edgeColor.x);
    ImGui::SliderFloat("Edge Color Intensity", &edgeColorIntensity, 0.0f, 1.0f);
    ImGui::Checkbox("Enable Color Blending", reinterpret_cast<bool*>(&enableColorBlending));

    ImGui::PopID();
#endif // _DEBUG

}
