#include "PSOFactory.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOFlags.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/RootSignatureBuilder/RootSignatureBuilder.h>
#include <Debug/Debug.h>
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
}

Microsoft::WRL::ComPtr<ID3D12PipelineState>
PSOFactory::CreateGraphicsPSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
        &_desc, IID_PPV_ARGS(&pso));

    if (FAILED(hr))
    {
        Debug::Log("Failed to create Graphics PSO\n");
        return nullptr;
    }

    return pso;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& _desc)
{
    // 互換性のために残すが、基本はRootSignatureBuilderの使用を推奨
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&_desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                             &signatureBlob, &errorBlob);

    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    hr = dxCommon_->GetDevice()->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    return rootSignature;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>PSOFactory::CreateModelRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)    // [0] Camera (VS/PS)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_VERTEX) // [1] TransformationMatrix (VS)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_PIXEL)  // [2] gMaterial (PS)
        .AddCBV(2, D3D12_SHADER_VISIBILITY_PIXEL)  // [3] gColor (PS)
        .AddSRVTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL) // [4] gTexture (t0)
        .AddCBV(3, D3D12_SHADER_VISIBILITY_ALL)    // [5] gLightGroup (VS/PS)
        .AddSRVTable(1, 1, D3D12_SHADER_VISIBILITY_PIXEL) // [6] gShadowMap (t1)
        .AddSRVTable(1, 2,
                     D3D12_SHADER_VISIBILITY_PIXEL) // [7] gPointLightShadowMap (t2)
        .AddSRVTable(1, 3,
                     D3D12_SHADER_VISIBILITY_PIXEL) // [8] gEnviromentTexture (t3)
        .AddDefaultStaticSampler(0)                 // gSampler (s0)
        .AddDepthComparisonStaticSampler(1)         // gShadowSampler (s1)
        .AddDefaultStaticSampler(2) // gPointLightShadowSampler (s2)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateSpriteRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)   // affineMat/uvTransMat (b0)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_PIXEL) // gColor (b1)
        .AddSRVTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL) // gTexture (t0)
        .AddDefaultStaticSampler(0)                       // gSampler (s0)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateParticleRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX) // gViewProjection (b0)
        .AddSRVTable(
            1, 0,
            D3D12_SHADER_VISIBILITY_VERTEX) // gParticle (t0, StructuredBuffer)
        .AddSRVTable(1, 0,
                     D3D12_SHADER_VISIBILITY_PIXEL) // gTexture (t0, Texture2D)
        .AddDefaultStaticSampler(0)                 // gSampler (s0)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateOffScreenRootSignature()
{
    return RootSignatureBuilder()
        .AddSRVTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL) // gTexture (t0)
        .AddDefaultStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                 D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                 D3D12_SHADER_VISIBILITY_ALL)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateSkyBoxRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX) // TransformationMatrix (b0)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_VERTEX) // Camera (b1)
        .AddCBV(2, D3D12_SHADER_VISIBILITY_PIXEL)  // gColor (b2)
        .AddSRVTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL) // gSkyBoxTexture (t0)
        .AddDefaultStaticSampler(0)                       // gSkyBoxSampler (s0)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateShadowMapRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX) // TransformationMatrix (b0)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_VERTEX) // gLightGroup (b1)
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreatePLShadowMapRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX) // TransformationMatrix (b0)
        .AddCBV(1, D3D12_SHADER_VISIBILITY_ALL)    // gLightGroup (b1) - VS(GS)とPSの両方で使用
        .Build();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature>
PSOFactory::CreateTextRootSignature()
{
    return RootSignatureBuilder()
        .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)            // ViewProjection (b0)
        .AddSRVTable(1, 0, D3D12_SHADER_VISIBILITY_VERTEX) // worldMatrices (t0,
        // StructuredBuffer)
        .AddSRVTable(1, 1, D3D12_SHADER_VISIBILITY_PIXEL)  // fontTexture (t1)
        .AddDefaultStaticSampler(0)                        // fontSampler (s0)
        .Build();
}

D3D12_BLEND_DESC PSOFactory::CreateBlendDesc(PSOFlags::BlendMode _mode)
{
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;

    switch (_mode)
    {
        case PSOFlags::BlendMode::Normal:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case PSOFlags::BlendMode::Add:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case PSOFlags::BlendMode::Sub:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case PSOFlags::BlendMode::Multiply:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case PSOFlags::BlendMode::Screen:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case PSOFlags::BlendMode::PremultipliedAdd:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        default:
            blendDesc.RenderTarget[0].BlendEnable = FALSE;
            break;
    }
    return blendDesc;
}

D3D12_RASTERIZER_DESC
PSOFactory::CreateRasterizerDesc(PSOFlags::CullMode _mode)
{
    D3D12_RASTERIZER_DESC desc{};
    desc.DepthClipEnable = true;
    desc.FillMode = D3D12_FILL_MODE_SOLID;
    switch (_mode)
    {
        case PSOFlags::CullMode::None:
            desc.CullMode = D3D12_CULL_MODE_NONE;
            break;
        case PSOFlags::CullMode::Front:
            desc.CullMode = D3D12_CULL_MODE_FRONT;
            break;
        case PSOFlags::CullMode::Back:
            desc.CullMode = D3D12_CULL_MODE_BACK;
            break;
        default:
            break;
    }
    return desc;
}

D3D12_DEPTH_STENCIL_DESC
PSOFactory::CreateDepthStencilDesc(PSOFlags::DepthMode _mode)
{
    D3D12_DEPTH_STENCIL_DESC desc{};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    desc.StencilEnable = false;

    switch (_mode)
    {
        case PSOFlags::DepthMode::Disable:
            desc.DepthEnable = false;
            break;
        case PSOFlags::DepthMode::Comb_mZero_fLessEqual:
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            break;
        case PSOFlags::DepthMode::Comb_mAll_fGreater:
            desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            break;
        case PSOFlags::DepthMode::Comb_mZero_fAlways:
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            break;
        default:
            break;
    }
    return desc;
}

} // namespace Engine
