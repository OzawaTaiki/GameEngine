#pragma once

#include "PostEffectBase.h"

#include <Math/Vector/Vector2.h>

struct BloomConstantBufferData
{
    float threshold = 1.0f;      // ブルームの閾値     標準値1.0f
    float intensity = 1.0f;      // ブルームの強度     標準値1.0f
    float softKnee = 0.5f;       // ソフトニー値      標準値0.5f
    float pad2;
};

struct BloomBlurConstantBufferData
{
    Vector2 texelSize;  // テクセルサイズ
    float blurRadius;
    float pad;
};

class Bloom : public PostEffectBase
{
public:
    Bloom() = default;
    ~Bloom() override = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override;

    void UpdateData(BloomConstantBufferData bloomData);
    void UpdateData(BloomBlurConstantBufferData blurData);

    /// <summary>
    /// 適用処理
    /// </summary>
    /// <param name="input">入力テクスチャ名</param>
    /// <param name="output">出力テクスチャ名</param>
    void Apply(const std::string& input, const std::string& output) override;

private:
    void CreatePipelineStates();
    void CreateBuffers();


    void CreateBrightExtractPso();
    void CreateDownscalePso();
    void CreateHorizontalBlurPso();
    void CreateVerticalBlurPso();
    void CreateCombinePso();

    void ApplyBrightExtract(const std::string& input, const std::string& output);
    void ApplyDownscale(const std::string& input, const std::string& output);
    void ApplyBlur(const std::string& input, const std::string& output, bool horizontal);
    void ApplyCombine(const std::string& input, const std::string& bloom, const std::string& output);
private:

    struct Set
    {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
    };

    Set brightExtractSet_;
    Set downscaleSet_;
    Set horizontalBlurSet_;
    Set verticalBlurSet_;
    Set combineSet_;

    BloomConstantBufferData* bloomCBData_; // ブルーム用定数バッファデータ
    BloomBlurConstantBufferData* bloomBlurCBData_; // ブルームブラー用定数バッファデータ

    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_; // ブルーム用定数バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> blurConstantBuffer_; // ブルームブラー用定数バッファ

};
