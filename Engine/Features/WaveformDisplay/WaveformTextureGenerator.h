#pragma once

#include <vector>
#include <map>

#include <wrl.h>
#include <d3dx12.h>

class WaveformTexturesGenerator
{
public:

    WaveformTexturesGenerator() = default;
    ~WaveformTexturesGenerator() = default;

    void Initialize();

    void GenerateWaveformTexture(const std::vector<float>& _waveformData, uint32_t _textureHandle);

    uint32_t CreateWaveformTexture();



private:

    void CreateConstantBuffer();
    void CreateStructuredBuffer();
    void CreateComputePipeline();

private:

    struct WaveformParams {
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t segmentHeght;
        uint32_t waveformDataCount;
        
    };
    WaveformParams* params_;
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;

    const uint32_t waveformDataCount_ = 2048u * 6u; // 6セグメント分

    Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipeline_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    std::map<uint32_t, ID3D12Resource*> textureSRVMap_;

    Microsoft::WRL::ComPtr<ID3D12Resource> structuredBufferWaveform_;
    float* mappedWaveformData_ = nullptr;
    uint32_t srvIndexWaveformData_ = 0;


    static const uint32_t textureHeight_;
    static const uint32_t textureWidth_;
};