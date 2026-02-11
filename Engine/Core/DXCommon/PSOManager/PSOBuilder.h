#pragma once

#include "PSOflags.h"

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include <string>
#include <vector>


class PSOBuilder
{
public:
    static PSOBuilder Create();

    // シェーダ設定
    PSOBuilder& SetShaders(const std::string& _vsName, const std::string& _psName);
    PSOBuilder& SetVertexShader(const std::string& _name);
    PSOBuilder& SetPixelShader(const std::string& _name);
    PSOBuilder& SetGeometryShader(const std::string& _name);

    // PSOFlags から一括設定
    PSOBuilder& SetFlags(PSOFlags _flags);

    // 個別設定
    PSOBuilder& SetBlendMode(PSOFlags::BlendMode _mode);
    PSOBuilder& SetCullMode(PSOFlags::CullMode _mode);
    PSOBuilder& SetDepthMode(PSOFlags::DepthMode _mode);

    // 細かい設定
    PSOBuilder& SetDepthWrite(bool _enable);
    PSOBuilder& SetDepthTest(bool _enable);
    PSOBuilder& SetWireframe(bool _enable);

    // RootSignature
    PSOBuilder& SetRootSignature(ID3D12RootSignature* _rootSig);
    PSOBuilder& UseModelRootSignature();  // Model用RootSigを自動取得

    // InputLayout
    PSOBuilder& UseModelInputLayout();  // Model用InputLayoutを設定
    PSOBuilder& UseFullScreenInputLayout(); // フルスクリーン用InputLayoutを設定
    PSOBuilder& UseLineDrawerInputLayout(); // LineDrawer用InputLayoutを設定
    PSOBuilder& UseParticleInputLayout(); // Particle用InputLayoutを設定
    PSOBuilder& UseTextInputLayout(); // Text3DRenderer用InputLayoutを設定
    PSOBuilder& UseSpriteInputLayout(); // Sprite用InputLayoutを設定
    PSOBuilder& SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& _inputElements);

    // Topology
    PSOBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _type);

    // RTV/DSV Format
    PSOBuilder& SetRTVFormat(DXGI_FORMAT _format);
    PSOBuilder& SetDSVFormat(DXGI_FORMAT _format);

    // ビルド
    Microsoft::WRL::ComPtr<ID3D12PipelineState> Build();
    Microsoft::WRL::ComPtr<ID3D12PipelineState> Build(ID3D12RootSignature* rootSignature);

    // ビルド + 登録
    Microsoft::WRL::ComPtr<ID3D12PipelineState> BuildAndRegister(const std::string& _name);


private:
    PSOBuilder();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_;

    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> gsBlob_;

    ID3D12RootSignature* rootSignature_ = nullptr;

    bool hasVertexShader_ = false;
    bool hasPixelShader_ = false;
    bool hasRootSignature_ = false;

    void ApplyDefaults();  // デフォルト値設定
    void Validate();       // エラーチェック
};