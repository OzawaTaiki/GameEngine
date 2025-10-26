#include "SpectrumTextureGenerator.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Debug/Debug.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

const size_t SpectrumTextureGenerator::kMaxSpectrumDataCount = 1024;
const float SpectrumTextureGenerator::kDefaultWidth = 14.0f;// w1024 64本描画すると仮定
const float SpectrumTextureGenerator::kDefaultMargin = 2.0f;// w1024 64本描画すると仮定


SpectrumTextureGenerator::SpectrumTextureGenerator(uint32_t _textureWidth, uint32_t _textureHeight) :
    textureWidth_(_textureWidth),
    textureHeight_(_textureHeight),
    width_(kDefaultWidth),
    margin_(kDefaultMargin)
{
}

void SpectrumTextureGenerator::Initialize(const Vector4& _backColor)
{
    CreateRootSignature();
    CreatePipelineState();

    CreateBuffers(_backColor);
    MakeLogRanges(44100.0f);
}

void SpectrumTextureGenerator::Generate(const std::vector<float>& _spectrumData, float _rms, int32_t _drawCount)
{
    CalculateWidthAndMargin(_drawCount);

    DXCommon* dxCommon = DXCommon::GetInstance();
    ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

    // 定数バッファ更新
    cbData_->textureWidth = textureWidth_;
    cbData_->textureHeight = textureHeight_;
    //if (!_spectrumData.empty()) // 空なら更新しない
        //cbData_->maxMagnitude = *std::max_element(_spectrumData.begin(), _spectrumData.end());
    cbData_->dataCount = static_cast<uint32_t>(std::min(_spectrumData.size(), kMaxSpectrumDataCount));
    cbData_->drawCount = _drawCount;
    cbData_->width = width_;
    cbData_->margin = margin_;
    cbData_->rms = _rms;

    // ストラクチャードバッファ更新
    size_t copySize = sizeof(float) * std::min(_spectrumData.size(), kMaxSpectrumDataCount);
    memcpy(spectrumData_, _spectrumData.data(), copySize);


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
    commandList->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(rangesSrvIndex_));

    // 描画コマンド
    commandList->DrawInstanced(6, cbData_->drawCount, 0, 0);

    // リソースバリア
    // テクスチャとして扱えるようにする
    renderTexture_->ChangeRTVState(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    renderTexture_->CancelClear();
}

D3D12_GPU_DESCRIPTOR_HANDLE SpectrumTextureGenerator::GetTextureGPUHandle() const
{
    return renderTexture_->GetGPUHandleofRTV();
}

void  SpectrumTextureGenerator::MakeLogRanges(float sampleRate,  float fmin, float fmax, int32_t bars, int32_t fftBins, int32_t fftSize)
{
    // 同じ条件なら再計算しない
    if (cashedDrawData_.CashEquals(bars, fftBins, sampleRate, fmin, fmax))
        return;



    if (bars > 128)
    {
        Debug::Log("Max bars is 128\n");
        bars = 128;
    }


    float logFmin = std::log(fmin);
    float logFmax = std::log(fmax);
    Debug::Log("============\n");
    for (int32_t b = 0; b < bars; ++b)
    {
            float p0 = (float)b / bars;
        float p1 = (float)(b + 1) / bars;
        float f0 = std::exp(logFmin + (logFmax - logFmin) * p0);
        float f1 = std::exp(logFmin + (logFmax - logFmin) * p1);

        int32_t i0 = (int32_t)std::floor(f0 * fftSize / sampleRate);
        int32_t i1 = (int32_t)std::ceil(f1 * fftSize / sampleRate);

        if (i0 < 0) i0 = 0;
        if (i1 >= fftBins) i1 = fftBins - 1;
        if (i1 < i0) i1 = i0;

        Debug::Log(std::format("Range[{}] : min_{},max_{}\n", b, i0, i1));

        ranges_[b] = { i0, i1 };

    }

    //cashedDrawData_.drawCount = bars;
    cashedDrawData_.fftBins = fftBins;
    cashedDrawData_.sampleRate = sampleRate;
    cashedDrawData_.minHz = fmin;
    cashedDrawData_.maxHz = fmax;
}

void SpectrumTextureGenerator::ReserveClear()
{
    renderTexture_->ReserveClear();
}

void SpectrumTextureGenerator::CalculateWidthAndMargin(int32_t _drawCount)
{
    if (_drawCount == 0)
        return;

    if (cashedDrawData_.drawCount == _drawCount)
        return;

    const int32_t defaultDrawCount = 64;

    float scale = static_cast<float>(_drawCount) / static_cast<float>(defaultDrawCount);
    width_ = kDefaultWidth / scale;
    margin_ = kDefaultMargin / scale;

    cashedDrawData_.drawCount = _drawCount;
}

void SpectrumTextureGenerator::CreateBuffers(const Vector4& _backColor)
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    SRVManager* srvManager = SRVManager::GetInstance();

    constantBuffer_ = dxCommon->CreateBufferResource(sizeof(ConstantBufferData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cbData_));


    dataSrvIndex_ = srvManager->Allocate();
    spectrumDataBuffer_ = dxCommon->CreateBufferResource(sizeof(float) * kMaxSpectrumDataCount);
    spectrumDataBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&spectrumData_));
    srvManager->CreateSRVForStructureBuffer(dataSrvIndex_, spectrumDataBuffer_.Get(), kMaxSpectrumDataCount, sizeof(float));

    // Range用バッファ
    rangesSrvIndex_ = srvManager->Allocate();
    rangesBuffer_ = dxCommon->CreateBufferResource(sizeof(Range) * 128);
    rangesBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&ranges_));
    srvManager->CreateSRVForStructureBuffer(rangesSrvIndex_, rangesBuffer_.Get(), 128, sizeof(Range));


    uint32_t handle  = RTVManager::GetInstance()->CreateRenderTarget(
        "SpectrumTexture",
        textureWidth_,
        textureHeight_,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        _backColor,
        false
    );

    renderTexture_ = RTVManager::GetInstance()->GetRenderTexture(handle);
    textureHandle_ = TextureManager::GetInstance()->GetTextureHandle(
        "SpectrumTexture",
        renderTexture_->GetSRVIndex(),
        renderTexture_->GetGPUHandleofRTV());
}

void SpectrumTextureGenerator::CreateRootSignature()
{
    DXCommon* dxCommon = DXCommon::GetInstance();
    HRESULT hr = S_FALSE;

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    descriptorRange[1].BaseShaderRegister = 1;
    descriptorRange[1].NumDescriptors = 1;//数は１つ
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // transform
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;


    // テクスチャ
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // range
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;


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
