#include "SpectrumTextureGenerator.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Debug/Debug.h>

const size_t SpectrumTextureGenerator::kMaxSpectrumDataCount = 4096;

SpectrumTextureGenerator::SpectrumTextureGenerator(uint32_t _textureWidth, uint32_t _textureHeight):
    textureWidth_(_textureWidth),
    textureHeight_(_textureHeight),
    width_(24.0f),// w1024 32本描画すると仮定
    margin_(8.0f) // w1024 32本描画すると仮定
{
}

void SpectrumTextureGenerator::Initialize()
{
    CreateRootSignature();
    CreatePipelineState();

    CreateBuffers();
}

void SpectrumTextureGenerator::Generate(const std::vector<float>& _spectrumData, float _maxMagnitude, uint32_t _drawCount)
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

    // 定数バッファ更新
    cbData_->textureWidth = textureWidth_;
    cbData_->textureHeight = textureHeight_;
    cbData_->maxMagnitude = _maxMagnitude;
    cbData_->dataCount = static_cast<uint32_t>(std::min(_spectrumData.size(), kMaxSpectrumDataCount));
    cbData_->drawCount = _drawCount;
    cbData_->width = width_;
    cbData_->margin = margin_;

    // ストラクチャードバッファ更新
    size_t copySize = sizeof(float) * std::min(_spectrumData.size(), kMaxSpectrumDataCount);
    memcpy(spectrumData_, _spectrumData.data(), copySize);
    spectrumDataBuffer_->Unmap(0, nullptr);
    spectrumDataBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&spectrumData_));

    /// 描画コマンド
    // パイプラインステートとルートシグネチャの設定
    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    // プリミティブ形状を設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // レンダーターゲットの設定
    renderTexture_->SetRenderTexture();

    // bufferの設定
    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(dataSrvIndex_));

    // 描画コマンド
    commandList->DrawInstanced(6, cbData_->drawCount, 0, 0);

    // リソースバリア
    // テクスチャとして扱えるようにする
    renderTexture_->ChangeRTVState(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_GPU_DESCRIPTOR_HANDLE SpectrumTextureGenerator::GetTextureHandle() const
{
    return renderTexture_->GetGPUHandleofRTV(); 
}

void SpectrumTextureGenerator::CreateBuffers()
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    SRVManager* srvManager = SRVManager::GetInstance();

    constantBuffer_ = dxCommon->CreateBufferResource(sizeof(ConstantBufferData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cbData_));



    dataSrvIndex_ = srvManager->Allocate();
    spectrumDataBuffer_ = dxCommon->CreateBufferResource(sizeof(float) * kMaxSpectrumDataCount);
    spectrumDataBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&spectrumData_));
    srvManager->CreateSRVForStructureBuffer(dataSrvIndex_, spectrumDataBuffer_.Get(), kMaxSpectrumDataCount, sizeof(float));


    uint32_t handle  = RTVManager::GetInstance()->CreateRenderTarget(
        "SpectrumTexture",
        textureWidth_,
        textureHeight_,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        Vector4(0.0f, 0.0f, 0.0f, 0.0f),
        false
    );

    renderTexture_ = RTVManager::GetInstance()->GetRenderTexture(handle);

}

void SpectrumTextureGenerator::CreateRootSignature()
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    HRESULT hr = S_FALSE;

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[2] = {};

    // transform
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;


    // テクスチャ
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void SpectrumTextureGenerator::CreatePipelineState()
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    PSOManager* psoManager = PSOManager::GetInstance();

    HRESULT hr =S_FALSE;

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = psoManager->ComplieShader(L"SpectrumTextureGenerator.hlsl", L"vs_6_0", L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = psoManager->ComplieShader(L"SpectrumTextureGenerator.hlsl", L"ps_6_0", L"PSmain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc = PSOManager::GetDepthStencilDesc(PSOFlags::DepthMode::Disable);
#pragma endregion


#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = PSOManager::GetBlendDesc(PSOFlags::BlendMode::Normal);
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = PSOManager::GetRasterizerDesc(PSOFlags::CullMode::None);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();                                         // RootSignature
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
    hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

/*

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // transform
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // camera
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 1;

    // color
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].Descriptor.ShaderRegister = 2;

    // テクスチャ
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数



    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    uint64_t type = flag.GetTypeValue();

    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion


#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"SkyBox.hlsl", L"vs_6_0",L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"SkyBox.hlsl", L"ps_6_0", L"PSmain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(flag);
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);

    //三角形を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
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
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));


    graphicsPipelineStates_[static_cast<uint64_t>(flag)] = pipelineState;
*/