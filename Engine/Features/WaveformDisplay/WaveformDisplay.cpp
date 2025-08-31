#include "WaveformDisplay.h"

#include <System/Audio/SoundInstance.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Debug/Debug.h>
#include <Math/MyLib.h>
#include <numbers>

float WaveformDisplay::waveformTimeWindow_ = 5.0f;

void WaveformDisplay::Initialize(const SoundInstance* _soundInstance, const WaveformBounds& _bounds)
{
    soundInstance_ = _soundInstance;
    bounds_ = _bounds;

    SetSampleRate(_soundInstance->GetSampleRate());

    lineDrawer_ = LineDrawer::GetInstance();

    startTime_ = 0.0f; // デフォルトの開始時間
    endTime_ = 0.0f;   // デフォルトの終了時間
}

void WaveformDisplay::Draw()
{
    if (!soundInstance_ || !lineDrawer_ || soundInstance_->GetWaveform().empty())
    {
        return; // 初期化されていない場合は何もしない
    }

    CalculateDisplayRange();

    DrawCenterLine();

    DrawWaveform();
}

void WaveformDisplay::SetDisplayTimeWindow(float _displayTimeWindow)
{
    displayTimeWindow_ = _displayTimeWindow;

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::SetStartTime(float _startTime)
{
    startTime_ = _startTime;

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::SetSampleRate(float _sampleRate)
{
    sampleRate_ = _sampleRate;

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::SetSoundInstance(const SoundInstance* _soundInstance)
{
    soundInstance_ = _soundInstance;
    SetSampleRate(_soundInstance->GetSampleRate());

    waveformCache_ = soundInstance_->GetWaveform();

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::CalculateDisplayRange()
{
    if (!soundInstance_ || isValidCache_)
    {
        return; // soundInstanceが設定されていない場合は何もしない
    }


    endTime_ = std::min(soundInstance_->GetDuration(), startTime_ + displayTimeWindow_);

    auto resampled = LanczosResample(
        waveformCache_,
        static_cast<size_t>(startTime_ * sampleRate_),
        static_cast<size_t>(endTime_ * sampleRate_),
        static_cast<int>(bounds_.size.x)
    );

    // 波形データを計算しなおす VBVにね
    ZeroMemory(mappedVertexBuffer_, sizeof(Vector2) * kMaxVertices_);

    for (size_t i = 0; i < resampled.size(); ++i)
    {
        mappedVertexBuffer_[i] = { static_cast<float>(i),resampled[i] };
    }


    isValidCache_ = true;

}

std::vector<float> WaveformDisplay::LanczosResample(const std::vector<float>& _input, int _outputSize ) const
{
    std::vector<float> output;
    output.reserve(_outputSize);

    float ratio = static_cast<float>(_input.size()) / _outputSize;

    const int lanczosKernelSize = 3; // ランツォスカーネルのサイズ

    for (int i = 0; i < _outputSize; ++i)
    {
        float x = i * ratio;
        int xInt = static_cast<int>(x);

        float xFrac = x - xInt;
        float sum = 0.0f;
        float weightSum = 0.0f;

        for (int j = -lanczosKernelSize + 1; j <= lanczosKernelSize; ++j)
        {
            int sampleIndex = xInt + j;
            if (sampleIndex >= 0 && sampleIndex < _input.size())
            {
                float kernelValue = LanczosKernel(x - sampleIndex, lanczosKernelSize);
                sum += _input[sampleIndex] * kernelValue;
                weightSum += kernelValue;
            }
        }

        output.push_back(weightSum > 0 ? sum / weightSum : 0.0f);
    }

    return output; // ランツォス再サンプリングされた波形データを返す

}

std::vector<float> WaveformDisplay::LanczosResample(const std::vector<float>& _input, size_t _startSample, size_t _enbSample, int _outputSize) const
{
    std::vector<float> clampedInput(
        _input.begin() + static_cast<size_t>(_startSample),
        _input.begin() + static_cast<size_t>(std::min(_enbSample, static_cast<size_t>(_input.size())))
    );

    return LanczosResample(clampedInput, _outputSize);

}

float WaveformDisplay::LanczosKernel(float _x, int _kernelSize) const
{
    if (_x == 0.0f)
        return 1.0f; // ランツォスカーネルの定義
    if (std::abs(_x) >= _kernelSize)
        return 0.0f; // カーネル外の値は0

    float piX = std::numbers::pi_v<float> * _x;
    float piXKernel = piX / _kernelSize;

    return std::sin(piX) / piX * std::sin(piXKernel) / piXKernel; // ランツォスカーネルの計算
}


void WaveformDisplay::DrawCenterLine()
{
    float centerY = bounds_.CenterY();

    lineDrawer_->RegisterPoint(Vector2(bounds_.leftTop.x, centerY), Vector2(bounds_.Right(), centerY), Vector4(1, 1, 1, 1));

}

void WaveformDisplay::DrawWaveform()
{
    if (waveformCache_.empty() || !isValidCache_)
    {
        waveformCache_ = soundInstance_->GetWaveform(startTime_, endTime_);
        waveformCache_ = LanczosResample(waveformCache_, static_cast<int>(bounds_.size.x));
        isValidCache_ = true;
    }

    float duration = soundInstance_->GetDuration();

    if (duration <= 0.0f)
    {
        return; // 無効な再生時間の場合は何もしない
    }

    int totalSamples = static_cast<int>(waveformCache_.size());
    int displayWidth = static_cast<int>(bounds_.size.x);

    if (totalSamples == 0 || displayWidth == 0) return;

    // 上下の余白
    const float verticalMargin = 0.1f; // 10%の余白

    float top = bounds_.leftTop.y + verticalMargin;
    float bottom = bounds_.Bottom() - verticalMargin;

    float sampleRate = soundInstance_->GetSampleRate();

    size_t startIndex = static_cast<size_t>(std::floor(startTime_ * sampleRate));
    size_t endIndex = static_cast<size_t>(std::ceil(endTime_ * sampleRate));

    float currentTime = static_cast<float>(startIndex) / sampleRate;
    currentTime = std::clamp(currentTime, startTime_, endTime_); // 時間を表示範囲に制限

    float currentX = bounds_.leftTop.x + (currentTime - startTime_) / displayTimeWindow_ * bounds_.size.x;
    float currentY = Lerp(bottom, top, (waveformCache_[startIndex] + 1.0f) / 2.0f); // 波形の値をY座標に変換
    float timeStep = static_cast<float>(soundInstance_->GetDuration() / waveformCache_.size());
    for (size_t i = 0;  i < waveformCache_.size(); ++i)
    {
        float nextTime = static_cast<float>((i + 1) * timeStep) / sampleRate; // サンプルの時間

        nextTime = std::clamp(nextTime, startTime_, endTime_); // 時間を表示範囲に制限

        float x = bounds_.leftTop.x + static_cast<float>(i);
        float y = Lerp(bottom, top, (waveformCache_[i] + 1.0f) / 2.0f); // 波形の値をY座標に変換

        lineDrawer_->RegisterPoint(Vector2(currentX, currentY), Vector2(x, y), Vector4(0.3f, 0.5f, 1.0f, 1.0f));

        currentX = x;
        currentY = y;

    }
}

void WaveformDisplay::CreatePipeline()
{
    HRESULT hr = S_FALSE;

    auto PSOManager = PSOManager::GetInstance();

    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;



    D3D12_ROOT_PARAMETER rootParameters[1] = {};

    // コンスタントバッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.RegisterSpace = 0;

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
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));


#pragma region DepthStencilState
        //DepthStencilStateの設定
        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager->ComplieShader(L"WaveformGenerator.hlsl", L"vs_6_0", L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager->ComplieShader(L"WaveformGenerator.hlsl", L"ps_6_0", L"PSmain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc = PSOManager->GetBlendDesc(PSOFlags::BlendMode::Normal);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = PSOManager->GetRasterizerDesc(PSOFlags::CullMode::None);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();                                         // RootSignature
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
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = DXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));

}

void WaveformDisplay::CreateConstantBuffer()
{
    constantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapData_));

    mapData_->color = Vector4(0.3f, 0.5f, 1.0f, 1.0f);
    mapData_->displayDuration = displayTimeWindow_;

    mapData_->leftTop = bounds_.leftTop;
    mapData_->displayHeight = bounds_.size.y;
    mapData_->displayWidth = bounds_.size.x;
    mapData_->startTime = startTime_;

}

void WaveformDisplay::CreateVertexBuffer()
{
    vertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(Vector2) * kMaxVertices_);
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexBuffer_));

    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(Vector2) * kMaxVertices_;
    vertexBufferView_.StrideInBytes = sizeof(Vector2);
}
