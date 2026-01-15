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


namespace Engine {

DepthBasedOutLine::~DepthBasedOutLine()
{
    camera_ = nullptr;
}

void DepthBasedOutLine::Initialize()
{
    CreateConstantBuffer(sizeof(DepthBasedOutLineData));

    // PSOとルートシグネチャを生成
    CreateRootSignature();
    CreatePipelineState();
}

void DepthBasedOutLine::Apply(const std::string& _input, const std::string& _output)
{
    //auto cmdList = DXCommon::GetInstance()->GetCommandList();

    //UpdateData();

    //// パイプラインステート設定
    //cmdList->SetPipelineState(pipelineState_.Get());
    //cmdList->SetComputeRootSignature(rootSignature_.Get());

    //// ルートパラメータ設定
    //// CBV (b0) - 定数バッファ
    //cmdList->SetComputeRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

    //auto rtvManager = RTVManager::GetInstance();
    //UINT inputSRVIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofRTV();
    //uint32_t depthSRVIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofDSV();

    //rtvManager->GetRenderTexture(_input)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    //// SRV (t0) - 入力テクスチャ
    //cmdList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(inputSRVIndex));
    //// SRV (t1) 深度テクスチャ
    //cmdList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(depthSRVIndex));
    //// UAV (u0) - 出力テクスチャ

    //// スレッドグループ数計算 (8x8のスレッドグループ)
    //uint32_t dispatchX = (WinApp::kWindowWidth_ + 7) / 8;
    //uint32_t dispatchY = (WinApp::kWindowHeight_ + 7) / 8;

    //// Dispatch実行
    //cmdList->Dispatch(dispatchX, dispatchY, 1);


    //rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    //rtvManager->GetRenderTexture(_output)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    UpdateData();

    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    auto rtvManager = RTVManager::GetInstance();
    // パイプラインステート設定
    cmdList->SetPipelineState(pipelineState_.Get());
    cmdList->SetGraphicsRootSignature(rootSignature_.Get());

    uint32_t depthSRVIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofDSV();

    rtvManager->GetRenderTexture(_output)->SetRenderTexture();
    rtvManager->GetRenderTexture(_input)->ChangeRTVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    uint32_t srvIndex = rtvManager->GetRenderTexture(_input)->GetSRVindexofRTV();
    // ルートパラメータ設定
    // CBV (b0) - 定数バッファ
    cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

    cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));

    cmdList->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(depthSRVIndex));
    // Dispatch実行
    cmdList->DrawInstanced(3, 1, 0, 0);

    rtvManager->GetRenderTexture(_input)->ChangeDSVState(cmdList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

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

    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"FullScreen.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"DepthBasedOutline.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    D3D12_BLEND_DESC blendDesc{};
    blendDesc = PSOManager::GetInstance()->GetBlendDesc(PSOFlags::BlendMode::Normal);

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();                                                // RootSignature
    graphicsPipelineStateDesc.BlendState = blendDesc;                                                               // BlendState
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
}

void DepthBasedOutLine::CreateRootSignature()
{
    HRESULT hr = S_FALSE;

    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // 深度テクスチャ用サンプラー
    staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[1].ShaderRegister = 1; // s1
    staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

#pragma region RootSignature
    // RootSignatureを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute用


    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
    descriptorRange[0].BaseShaderRegister = 0;//０から始まる
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    descriptorRange[1].BaseShaderRegister = 1;//０から始まる
    descriptorRange[1].NumDescriptors = 1;//数は１つ
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    // RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // CBV (b0) - 定数バッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // テクスチャ
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];		//tableの中身の配列を指定
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;//tableで利用する数


    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];		//tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

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

} // namespace Engine
