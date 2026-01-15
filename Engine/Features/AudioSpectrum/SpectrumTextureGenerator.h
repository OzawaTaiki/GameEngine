#pragma once

#include <Math/Vector/Vector4.h>

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <wrl.h>


namespace Engine {

class RenderTarget;


class SpectrumTextureGenerator
{
public:
    SpectrumTextureGenerator(uint32_t _textureWidth = 1024, uint32_t _textureHeight = 512);
    ~SpectrumTextureGenerator() = default;


    void Initialize(const Vector4& _backColor = { 0.0f,0.0f ,0.0f, 0.3f });

    void Generate(const std::vector<float>& _spectrumData, float _rms, int32_t drawCount = 64);

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHandle() const;
    uint32_t GetTextureHandle() const { return textureHandle_; }

    void SetWidth(float _width) { width_ = _width; }
    void SetMargin(float _margin) { margin_ = _margin; }

    void MakeLogRanges(float sampleRate, float fmin=100.0f, float fmax=10000.0f, int32_t bars=48, int32_t fftBins=512, int32_t fftSize=1024);
    void ReserveClear();
private:


    struct ConstantBufferData
    {
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t dataCount; // 配列サイズ
        uint32_t drawCount; // 描画するバーの数

        float width; // バーの幅
        float margin;// バーの間隔
        float rms; // RMS値  スケールとして使う
        float pad;

    };

    struct Range
    {
        int32_t start;
        int32_t end;

        Range() = default;
        Range(int32_t _s, int32_t _e) : start(_s), end(_e) {}
        ~Range() = default;
    };

private:

    void CreateBuffers(const Vector4& _backColor);

    void CreateRootSignature();

    void CreatePipelineState();

    void CalculateWidthAndMargin(int32_t drawCount);


private:

    ///===========
    /// DXリソース関連
    ///===========

    // 定数バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    ConstantBufferData* cbData_ = {};

    // ストラクチャードバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> spectrumDataBuffer_;
    float* spectrumData_ = {};
    uint32_t dataSrvIndex_ = 0;

    // テクスチャ
    RenderTarget* renderTexture_ = 0;
    uint32_t textureHandle_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> rangesBuffer_;
    uint32_t rangesSrvIndex_ = 0;
    Range* ranges_ = nullptr;


    // パイプライン
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;


    /// ===========
    ///
    /// ===========

    static const float kDefaultWidth;
    static const float kDefaultMargin;
    static const size_t kMaxSpectrumDataCount;

    uint32_t textureWidth_;
    uint32_t textureHeight_;

    float width_    = 0.0f;
    float margin_   = 0.0f;

    struct DrawData
    {
        size_t drawCount;
        size_t fftBins;
        float sampleRate;
        float minHz;
        float maxHz;

        bool operator==(const DrawData& other) const
        {
            return
                drawCount   == other.drawCount &&
                fftBins     == other.fftBins &&
                sampleRate  == other.sampleRate &&
                minHz       == other.minHz &&
                maxHz       == other.maxHz;
        }
        bool CashEquals(size_t _drawCount, size_t _fftBins, float _sampleRate, float _minHz, float _maxHz) const
        {
            return *this == DrawData{ _drawCount, _fftBins, _sampleRate, _minHz, _maxHz };
        }
    };

    DrawData cashedDrawData_{};

};

} // namespace Engine
