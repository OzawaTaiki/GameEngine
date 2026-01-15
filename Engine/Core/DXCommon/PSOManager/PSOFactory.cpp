#include "PSOFactory.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Debug/Debug.h>
#include <d3dcompiler.h>
#include <cassert>


namespace Engine {

PSOFactory* PSOFactory::GetInstance()
{
    static PSOFactory instance;
    return &instance;
}

void PSOFactory::Initialize()
{
    dxCommon_ = DXCommon::GetInstance();
    assert(dxCommon_);
    Debug::Log("PSOFactory initialized\n");
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOFactory::CreateGraphicsPSO(
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
        &_desc, IID_PPV_ARGS(&pso)
    );
    if (FAILED(hr))
    {
        Debug::Log("Failed to create Graphics PSO\n");
        return nullptr;
    }
    return pso;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> PSOFactory::CreateRootSignature(
    const D3D12_ROOT_SIGNATURE_DESC& _desc)
{

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(
        &_desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    hr = dxCommon_->GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    );
    assert(SUCCEEDED(hr));

    return rootSignature;
}

D3D12_BLEND_DESC PSOFactory::CreateBlendDesc(PSOFlags::BlendMode _mode)
{
    // PSOManager::GetBlendDesc()の実装を使用
    PSOFlags flags(_mode);
    return PSOManager::GetBlendDesc(flags);
}

D3D12_RASTERIZER_DESC PSOFactory::CreateRasterizerDesc(PSOFlags::CullMode _mode)
{
    // PSOManager::GetRasterizerDesc()の実装を使用
    PSOFlags flags(_mode);
    return PSOManager::GetRasterizerDesc(flags);
}

D3D12_DEPTH_STENCIL_DESC PSOFactory::CreateDepthStencilDesc(PSOFlags::DepthMode _mode)
{
    // PSOManager::GetDepthStencilDesc()の実装を使用
    PSOFlags flags(_mode);
    return PSOManager::GetDepthStencilDesc(flags);
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC PSOFactory::CreateDefaultModelDesc(
    ID3D12RootSignature* _rootSignature,
    IDxcBlob* _vsBlob,
    IDxcBlob* _psBlob,
    PSOFlags _flags)
{

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = _rootSignature;
    desc.VS = { _vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize() };
    desc.PS = { _psBlob->GetBufferPointer(), _psBlob->GetBufferSize() };

    // InputLayout（モデル用: POSITION, TEXCOORD, NORMAL）
    static D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    desc.InputLayout.pInputElementDescs = inputElementDescs;
    desc.InputLayout.NumElements = _countof(inputElementDescs);

    desc.BlendState = CreateBlendDesc(_flags.GetBlendMode());
    desc.RasterizerState = CreateRasterizerDesc(_flags.GetCullMode());
    desc.DepthStencilState = CreateDepthStencilDesc(_flags.GetDepthMode());

    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.SampleDesc.Count = 1;
    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    return desc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC PSOFactory::CreateDefaultSpriteDesc(
    ID3D12RootSignature* _rootSignature,
    IDxcBlob* _vsBlob,
    IDxcBlob* _psBlob,
    PSOFlags _flags)
{

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = _rootSignature;
    desc.VS = { _vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize() };
    desc.PS = { _psBlob->GetBufferPointer(), _psBlob->GetBufferSize() };

    // InputLayout（スプライト用: POSITION, TEXCOORD）
    static D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    desc.InputLayout.pInputElementDescs = inputElementDescs;
    desc.InputLayout.NumElements = _countof(inputElementDescs);

    desc.BlendState = CreateBlendDesc(_flags.GetBlendMode());
    desc.RasterizerState = CreateRasterizerDesc(_flags.GetCullMode());
    desc.DepthStencilState = CreateDepthStencilDesc(_flags.GetDepthMode());

    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.SampleDesc.Count = 1;
    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    return desc;
}

} // namespace Engine
