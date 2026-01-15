#include "Dissolve.h"

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>


namespace Engine {

void Dissolve::Initialize()
{
    thresholdBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ThresholdBuffer));
    thresholdBuffer_->Map(0, nullptr, (void**)&thresholdData_);

    thresholdData_->threshold = 0.5f;
    thresholdData_->maskColor = { 0.0f, 0.0f, 0.0f };

    textureHandle_ = TextureManager::GetInstance()->Load("noise/noise0.png");
    CreatePSOForDissolve();
}

void Dissolve::Set()
{
    PSOManager::GetInstance()->SetRegisterPSO("Dissolve"); 
    PSOManager::GetInstance()->SetRegisterRootSignature("Dissolve");

    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    cmdList->SetGraphicsRootConstantBufferView(1, thresholdBuffer_->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
}

void Dissolve::CreatePSOForDissolve()
{
    HRESULT hr = S_FALSE;


    // ルートシグネチャのためのサンプラー設定
    D3D12_STATIC_SAMPLER_DESC samplers[1] = {};

    // 通常テクスチャ用サンプラー
    samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    samplers[0].ShaderRegister = 0; // s0
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


    // テクスチャSRVのためのディスクリプタレンジ
    D3D12_DESCRIPTOR_RANGE descriptorRanges[1] = {};

    // 通常テクスチャ用レンジ
    descriptorRanges[0].BaseShaderRegister = 0; // t0
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE descriptorRangesformask[1] = {};
    // 深度テクスチャ用レンジ
    descriptorRangesformask[0].BaseShaderRegister = 1; // t1
    descriptorRangesformask[0].NumDescriptors = 1;
    descriptorRangesformask[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangesformask[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ディスクリプタテーブルの作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // テクスチャ用ディスクリプタテーブル
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRanges;

    // カメラの逆射影行列
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 0;


    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangesformask;

    // ルートシグネチャ記述の設定
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = _countof(samplers);
    rootSignatureDesc.pStaticSamplers = samplers;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));

    if (FAILED(hr)) {
        assert(false && "Failed to create root signature");
        return;
    }

    // シェーダーのコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob =
        PSOManager::GetInstance()->ComplieShader(L"FullScreen.VS.hlsl", L"vs_6_0");
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob =
        PSOManager::GetInstance()->ComplieShader(L"Dissolve.hlsl", L"ps_6_0");

    // PSO記述子の設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature.Get();

    // シェーダー設定
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

    // ブレンド設定
    psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // ラスタライザー設定
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    // 深度設定 - フルスクリーンパスでは無効化
    psoDesc.DepthStencilState.DepthEnable = FALSE;

    // レンダーターゲット設定
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    // その他の設定
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;

    // PSO作成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    hr = DXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(
        &psoDesc, IID_PPV_ARGS(&pipelineState));

    if (FAILED(hr)) {
        assert(false && "Failed to create pipeline state");
        return;
    }

    // PSOManagerに登録
    PSOManager::GetInstance()->RegisterPSO("Dissolve", pipelineState.Get());
    PSOManager::GetInstance()->RegisterRootSignature("Dissolve", rootSignature.Get());
}

} // namespace Engine
