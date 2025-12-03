#pragma once

#include <Core/DXCommon/PSOManager/PSOFlags.h>

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>

class DXCommon;

class PSOFactory {
public:
    static PSOFactory* GetInstance();

    void Initialize();

    // PSO生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPSO(
        const D3D12_GRAPHICS_PIPELINE_STATE_DESC& _desc
    );

    // RootSignature生成
    Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(
        const D3D12_ROOT_SIGNATURE_DESC& _desc
    );

    // ヘルパー: 頻出設定を簡単に作成
    static D3D12_BLEND_DESC CreateBlendDesc(PSOFlags::BlendMode _mode);
    static D3D12_RASTERIZER_DESC CreateRasterizerDesc(PSOFlags::CullMode _mode);
    static D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(PSOFlags::DepthMode _mode);

    // モデル用のデフォルトPSO Desc生成
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultModelDesc(
        ID3D12RootSignature* _rootSignature,
        IDxcBlob* _vsBlob,
        IDxcBlob* _psBlob,
        PSOFlags _flags = PSOFlags::ForNormalModel()
    );

    // スプライト用のデフォルトPSO Desc生成
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultSpriteDesc(
        ID3D12RootSignature* _rootSignature,
        IDxcBlob* _vsBlob,
        IDxcBlob* _psBlob,
        PSOFlags _flags = PSOFlags::ForSprite()
    );

private:
    PSOFactory() = default;
    ~PSOFactory() = default;
    PSOFactory(const PSOFactory&) = delete;
    PSOFactory& operator=(const PSOFactory&) = delete;

    DXCommon* dxCommon_ = nullptr;
};
