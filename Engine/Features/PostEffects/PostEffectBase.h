#pragma once

#include <string>

#include <wrl.h>
#include <d3d12.h>

struct PostEffectBaseData
{
    // 継承して拡張するためのデータ構造
    // コンスタントバッファ用の構造体やテクスチャインデックスとか

    virtual ~PostEffectBaseData() = default;
};

class PostEffectBase
{
public:
    PostEffectBase() = default;
    virtual ~PostEffectBase() = default;

    virtual void Initialize() = 0;

    virtual void Apply(const std::string& _input, const std::string& _output) = 0;

    virtual void SetData(const PostEffectBaseData* data) {};

protected:

    // 共通処理用ヘルパー
    void CreateConstantBuffer(size_t dataSize);
    void UpdateConstantBuffer(const void* data, size_t size);

protected:

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_; // パイプラインステートオブジェクト
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_; // ルートシグネチャ

    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_; // 定数バッファ
    void* constantBufferData_ = nullptr; // 定数バッファのデータポインタ

    PostEffectBaseData* postEffectData_ = nullptr; // ポストエフェクトのデータ

};