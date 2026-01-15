#include "WaveformDisplay.h"

#include <System/Audio/SoundInstance.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Features/WaveformDisplay/WaveformAnalyzer.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Debug/Debug.h>
#include <Math/MyLib.h>

#include <numbers>


namespace Engine {

float WaveformDisplay::waveformTimeWindow_ = 5.0f;

void WaveformDisplay::Initialize(const SoundInstance* _soundInstance, const WaveformBounds& _bounds, const Matrix4x4& _matVP)
{
    bounds_ = _bounds;
    SetSoundInstance(_soundInstance);

    CreatePipeline();
    CreateConstantBuffer(_matVP);
    CreateVertexBuffer();
    float samapleRate = 120.0f;
    if (_soundInstance) samapleRate = _soundInstance->GetSampleRate();
    SetSampleRate(samapleRate);

    lineDrawer_ = LineDrawer::GetInstance();

    startTime_ = 0.0f; // デフォルトの開始時間
    endTime_ = 0.0f;   // デフォルトの終了時間

}

void WaveformDisplay::Draw()
{
    if (!soundInstance_ || !lineDrawer_)
    {
        return; // 初期化されていない場合は何もしない
    }

    CalculateDisplayRange();

    DrawCenterLine();
    DrawPlayheadLine();
    DrawWaveform();
}

void WaveformDisplay::SetDisplayTimeWindow(float _displayTimeWindow)
{
    displayTimeWindow_ = _displayTimeWindow;

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::SetStartTime(float _startTime)
{
    if (startTime_ == _startTime)
        return;

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
    if (!_soundInstance)
        return;

    if (soundInstance_ == _soundInstance)
        return;

    soundInstance_ = _soundInstance;
    SetSampleRate(_soundInstance->GetSampleRate());

    waveformCache_ = WaveformAnalyzer::ExtractRawWaveformMaxMin(soundInstance_, bounds_.size.x, displayTimeWindow_);


    //float musicDuration = soundInstance_->GetDuration();
    //float sizePerSec = std::ceil(bounds_.size.x / displayTimeWindow_); // 1秒あたりの
    //float size = std::ceil(sizePerSec * musicDuration);
    //waveformCache_ = LanczosResample(
    //    waveformCache_,
    //    static_cast<size_t>(0),
    //    static_cast<size_t>(musicDuration * sampleRate_),
    //    static_cast<int>(size)
    //);

    isValidCache_ = false; // キャッシュを無効化
}

void WaveformDisplay::CalculateDisplayRange()
{
    if (!soundInstance_ || isValidCache_)
    {
        return; // soundInstanceが設定されていない場合は何もしない
    }


    float musicDuration = soundInstance_->GetDuration();

    // playheadを中心にするために、表示範囲を調整
    float displayStartTime  = startTime_ - displayTimeWindow_ * 0.5f;
    float displayEndTime    = startTime_ + displayTimeWindow_ * 0.5f;

    float clampedDisplayStartTime = std::max(0.0f, displayStartTime);
    float clampedDisplayEndTime   = std::min(musicDuration, displayEndTime);

    endTime_ = std::min(musicDuration, startTime_ + displayTimeWindow_);

    float sizePerSec    = std::ceil(bounds_.size.x / displayTimeWindow_); // 1秒あたりの
    float size          = std::ceil(sizePerSec * musicDuration);

    size_t startIndex   = static_cast<size_t>(size / musicDuration * clampedDisplayStartTime);
    size_t endIndex     = static_cast<size_t>(size / musicDuration * clampedDisplayEndTime);

    // 波形データを計算しなおす VBVにね
    ZeroMemory(mappedVertexBuffer_, sizeof(Vector2) * kMaxVertices_);

    instanceCount_  = 0;
    float renderDuration    = clampedDisplayEndTime - clampedDisplayStartTime;    // 表示要素数
    float displayRatio      = renderDuration / displayTimeWindow_; // 表示比率
    size_t localX           = static_cast<size_t>(Lerp(bounds_.size.x, 0.0f, displayRatio));

    if (musicDuration - displayEndTime < 0)
        localX = 0;

    for (size_t i = startIndex * 2; i < endIndex * 2 && i < waveformCache_.size(); i += 2) // 配列要素はmax/minでニコイチなので*2する
    {
        mappedVertexBuffer_[instanceCount_] = { static_cast<float>(localX),waveformCache_[i] };
        mappedVertexBuffer_[instanceCount_ + 1] = { static_cast<float>(localX++),waveformCache_[i + 1] };
        instanceCount_ += 2;
    }

    isValidCache_ = true;

}

std::vector<float> WaveformDisplay::LanczosResample(const std::vector<float>& _input, int _outputSize) const
{
    std::vector<float> output;
    output.reserve(_outputSize);

    float ratio = static_cast<float>(_input.size()) / _outputSize;

    const int lanczosKernelSize = 3; // ランツォスカーネルのサイズ

    for (int i = 0; i < _outputSize; ++i)
    {
        float x = i * ratio;
        int xInt = static_cast<int>(x);

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

std::vector<float> WaveformDisplay::SimpleResample(const std::vector<float>& _input, int _outputSize) const
{
    std::vector<float> output;
    output.reserve(_outputSize);

    // 1indexあたりにいくつのサンプルがあるか
    size_t sampleSize = _input.size() / _outputSize;

    for (size_t i = 0; i < _outputSize; ++i)
    {
        float min = FLT_MAX;
        float max = -FLT_MAX;

        for (size_t j = 0; j < sampleSize; ++j)
        {
            size_t index = i * sampleSize + j;
            if (index < _input.size())
            {
                min = std::min(min, _input[index]);
                max = std::max(max, _input[index]);
            }
        }
        output.push_back(min);
        output.push_back(max);
    }


    return output;
}

std::vector<float> WaveformDisplay::SimpleResample(const std::vector<float>& _input, size_t _startSample, size_t _enbSample, int _outputSize) const
{
    std::vector<float> clampedInput(
        _input.begin() + static_cast<size_t>(_startSample),
        _input.begin() + static_cast<size_t>(std::min(_enbSample, static_cast<size_t>(_input.size())))
    );

    return SimpleResample(clampedInput, _outputSize);
}

float WaveformDisplay::LanczosKernel(float _x, int _kernelSize) const
{
    if (_x == 0.0f)
        return 1.0f; // ランツォスカーネルの定義
    if (std::abs(_x) >= _kernelSize)
        return 0.0f; // カーネル外の値は0

    float piX = std::numbers::pi_v<float> *_x;
    float piXKernel = piX / _kernelSize;

    return std::sin(piX) / piX * std::sin(piXKernel) / piXKernel; // ランツォスカーネルの計算
}


void WaveformDisplay::DrawCenterLine()
{
    float centerY = bounds_.CenterY();

    lineDrawer_->RegisterPoint(Vector2(bounds_.leftTop.x, centerY), Vector2(bounds_.Right(), centerY), Vector4(1, 1, 1, 1));

}

void WaveformDisplay::DrawPlayheadLine()
{
    float playheadX = bounds_.CenterX();
    lineDrawer_->RegisterPoint(Vector2(playheadX, bounds_.leftTop.y), Vector2(playheadX, bounds_.Bottom()), Vector4(1, 0, 0, 1));
}

void WaveformDisplay::DrawWaveform()
{
    // データの更新済みなので描画だけ行う

    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

    commandList->DrawInstanced(instanceCount_, instanceCount_ / 2, 0, 0);
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
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

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

void WaveformDisplay::CreateConstantBuffer(const Matrix4x4& _matVP)
{
    constantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapData_));

    mapData_->color = Vector4(0.3f, 0.5f, 1.0f, 1.0f);
    mapData_->displayDuration = displayTimeWindow_;

    mapData_->leftTop = bounds_.leftTop;
    mapData_->displayHeight = bounds_.size.y;
    mapData_->displayWidth = bounds_.size.x;
    mapData_->startTime = startTime_;
    mapData_->screenSize = WinApp::kWindowSize_;
    mapData_->matViewProj = _matVP;

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

} // namespace Engine
