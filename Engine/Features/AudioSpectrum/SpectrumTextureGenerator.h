#pragma once

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

class RenderTarget;

class SpectrumTextureGenerator
{
public:
    SpectrumTextureGenerator(uint32_t _textureWidth = 1024 , uint32_t _textureHeight = 512);
    ~SpectrumTextureGenerator() = default;


    void Initialize();

    void Generate(const std::vector<float>& _spectrumData, float _maxMagnitude, uint32_t drawCount = 32);

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() const;

    void SetWidth(float _width) { width_ = _width; }
    void SetMargin(float _margin) { margin_ = _margin; }
private:

    struct ConstantBufferData
    {
        uint32_t textureWidth;
        uint32_t textureHeight;
        float maxMagnitude;
        uint32_t dataCount; // 配列サイズ

        uint32_t drawCount; // 描画するバーの数
        float width; // バーの幅
        float margin;// バーの間隔

        float pad;

    };

private:

    void CreateBuffers();

    void CreateRootSignature();

    void CreatePipelineState();


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

    // パイプライン
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;


    /// ===========
    ///
    /// ===========

    static const size_t kMaxSpectrumDataCount;

    uint32_t textureWidth_;
    uint32_t textureHeight_;

    float width_    = 0.0f;
    float margin_   = 0.0f;

};