#include "Bloom.h"

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Core/DXCommon/PSOManager/PSOBuilder.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/RootSignatureBuilder/RootSignatureBuilder.h>
#include <Core/DXCommon/ShaderCompiler/ShaderCompiler.h>

void Bloom::Initialize()
{
    CreatePipelineStates();
    CreateBuffers();

    // 中間テクスチャを作成
    RTVManager::GetInstance()->CreateRenderTarget(
        "bloom_temp0",
        WinApp::kWindowWidth_>>1,
        WinApp::kWindowHeight_>>1,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        { 0.0f,0.0f ,0.0f ,1.0f },
        false);

    RTVManager::GetInstance()->CreateRenderTarget(
        "bloom_temp1",
        WinApp::kWindowWidth_>>1,
        WinApp::kWindowHeight_>>1,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        { 0.0f,0.0f ,0.0f ,1.0f },
        false);
}

void Bloom::Apply(const std::string& input, const std::string& output)
{
    auto manager = RTVManager::GetInstance();

    manager->GetRenderTexture(input)->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    auto temp0 = manager->GetRenderTexture("bloom_temp0");
    auto temp1 = manager->GetRenderTexture("bloom_temp1");

    // 明るい部分抽出
    temp0->ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    ApplyBrightExtract(input, "bloom_temp0");
    // 入力テクスチャをダウンスケール
    temp0->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    temp1->ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    ApplyDownscale("bloom_temp0", "bloom_temp1");
    // 水平ブラー
    temp0->ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    temp1->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    ApplyBlur("bloom_temp1", "bloom_temp0", true);
    // 垂直ブラー
    temp0->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    temp1->ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    ApplyBlur("bloom_temp0", "bloom_temp1", false);
    // 合成
    temp1->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    ApplyCombine(input, "bloom_temp1", output);
}

void Bloom::CreatePipelineStates()
{
    CreateBrightExtractPso();
    CreateDownscalePso();
    CreateHorizontalBlurPso();
    CreateVerticalBlurPso();
    CreateCombinePso();
}

void Bloom::CreateBuffers()
{
    CreateConstantBufferResource(sizeof(BloomConstantBufferData), constantBuffer_, (void**)&bloomCBData_);
    CreateConstantBufferResource(sizeof(BloomBlurConstantBufferData), blurConstantBuffer_, (void**)&bloomBlurCBData_);
}

void Bloom::UpdateData(BloomConstantBufferData bloomData)
{
    *bloomCBData_ = bloomData;
}

void Bloom::UpdateData(BloomBlurConstantBufferData blurData)
{
    *bloomBlurCBData_ = blurData;
}

void Bloom::CreateBrightExtractPso()
{
    ShaderCompiler::GetInstance()->Register("BrightExtract", L"BrightExtract.PS.hlsl", L"ps_6_0");

    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder
        .AddCBV(0)
        .AddSRVTable(1, 0)
        .AddDefaultStaticSampler(0);

    brightExtractSet_.rootSig = rootSigBuilder.Build();

    auto builder = PSOBuilder::Create();
    using Flag = PSOFlags;
    brightExtractSet_.pso =
        builder
        .SetBlendMode(Flag::BlendMode::Normal)
        .SetCullMode(Flag::CullMode::None)
        .SetDepthMode(Flag::DepthMode::Disable)
        .SetPixelShader("BrightExtract")
        .SetVertexShader("FullScreen_VS")
        .UseFullScreenInputLayout()
        .SetRootSignature(brightExtractSet_.rootSig.Get())
        .Build();
}

void Bloom::CreateDownscalePso()
{
    ShaderCompiler::GetInstance()->Register("Downscale", L"Downsample.hlsl", L"ps_6_0");

    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder
        .AddCBV(0)
        .AddSRVTable(1, 0)
        .AddDefaultStaticSampler(0);

    downscaleSet_.rootSig = rootSigBuilder.Build();

    auto builder = PSOBuilder::Create();

    using Flag = PSOFlags;
    downscaleSet_.pso =
        builder
        .SetBlendMode(Flag::BlendMode::Normal)
        .SetCullMode(Flag::CullMode::None)
        .SetDepthMode(Flag::DepthMode::Disable)
        .SetPixelShader("Downscale")
        .SetVertexShader("FullScreen_VS")
        .UseFullScreenInputLayout()
        .SetRootSignature(downscaleSet_.rootSig.Get())
        .Build();

}

void Bloom::CreateHorizontalBlurPso()
{
    ShaderCompiler::GetInstance()->Register("HorizontalBlur", L"HorizontalBlur.hlsl", L"ps_6_0");

    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder
        .AddCBV(0)
        .AddSRVTable(1, 0)
        .AddDefaultStaticSampler(0);

    horizontalBlurSet_.rootSig = rootSigBuilder.Build();

    auto builder = PSOBuilder::Create();
    using Flag = PSOFlags;
    horizontalBlurSet_.pso =
        builder
        .SetBlendMode(Flag::BlendMode::Normal)
        .SetCullMode(Flag::CullMode::None)
        .SetDepthMode(Flag::DepthMode::Disable)
        .SetPixelShader("HorizontalBlur")
        .SetVertexShader("FullScreen_VS")
        .UseFullScreenInputLayout()
        .SetRootSignature(horizontalBlurSet_.rootSig.Get())
        .Build();
}

void Bloom::CreateVerticalBlurPso()
{
    ShaderCompiler::GetInstance()->Register("VerticalBlur", L"VerticalBlur.hlsl", L"ps_6_0");
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder
        .AddCBV(0)
        .AddSRVTable(1, 0)
        .AddDefaultStaticSampler(0);

    verticalBlurSet_.rootSig = rootSigBuilder.Build();

    auto builder = PSOBuilder::Create();

    using Flag = PSOFlags;
    verticalBlurSet_.pso =
        builder
        .SetBlendMode(Flag::BlendMode::Normal)
        .SetCullMode(Flag::CullMode::None)
        .SetDepthMode(Flag::DepthMode::Disable)
        .SetPixelShader("VerticalBlur")
        .SetVertexShader("FullScreen_VS")
        .UseFullScreenInputLayout()
        .SetRootSignature(verticalBlurSet_.rootSig.Get())
        .Build();
}

void Bloom::CreateCombinePso()
{
    ShaderCompiler::GetInstance()->Register("BloomCombine", L"BloomCombine.hlsl", L"ps_6_0");
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder
        .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL) // Bloom定数バッファ
        .AddSRVTable(1, 0) // 元画像
        .AddSRVTable(1, 1) // ブルーム画像
        .AddDefaultStaticSampler(0);

    combineSet_.rootSig = rootSigBuilder.Build();

    auto builder = PSOBuilder::Create();
    using Flag = PSOFlags;

    combineSet_.pso =
        builder
        .SetBlendMode(Flag::BlendMode::Add)
        .SetCullMode(Flag::CullMode::None)
        .SetDepthMode(Flag::DepthMode::Disable)
        .SetPixelShader("BloomCombine")
        .SetVertexShader("FullScreen_VS")
        .UseFullScreenInputLayout()
        .SetRootSignature(combineSet_.rootSig.Get())
        .Build();
}

void Bloom::ApplyBrightExtract(const std::string& input, const std::string& output)
{

    RTVManager::GetInstance()->SetRenderTexture(output);

    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    cmdList->SetPipelineState(brightExtractSet_.pso.Get());
    cmdList->SetGraphicsRootSignature(brightExtractSet_.rootSig.Get());

    cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
    uint32_t srvIndex = RTVManager::GetInstance()->GetRenderTexture(input)->GetSRVindexofRTV();
    cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void Bloom::ApplyDownscale(const std::string& input, const std::string& output)
{
    RTVManager::GetInstance()->SetRenderTexture(output);

    auto cmdList = DXCommon::GetInstance()->GetCommandList();
    cmdList->SetPipelineState(downscaleSet_.pso.Get());
    cmdList->SetGraphicsRootSignature(downscaleSet_.rootSig.Get());

    cmdList->SetGraphicsRootConstantBufferView(0, blurConstantBuffer_->GetGPUVirtualAddress());
    uint32_t srvIndex = RTVManager::GetInstance()->GetRenderTexture(input)->GetSRVindexofRTV();
    cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void Bloom::ApplyBlur(const std::string& input, const std::string& output, bool horizontal)
{
    if (horizontal)
    {
        RTVManager::GetInstance()->SetRenderTexture(output);
        auto cmdList = DXCommon::GetInstance()->GetCommandList();
        cmdList->SetPipelineState(horizontalBlurSet_.pso.Get());
        cmdList->SetGraphicsRootSignature(horizontalBlurSet_.rootSig.Get());
        cmdList->SetGraphicsRootConstantBufferView(0, blurConstantBuffer_->GetGPUVirtualAddress());
        uint32_t srvIndex = RTVManager::GetInstance()->GetRenderTexture(input)->GetSRVindexofRTV();
        cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));
        cmdList->DrawInstanced(3, 1, 0, 0);
    }
    else
    {
        RTVManager::GetInstance()->SetRenderTexture(output);
        auto cmdList = DXCommon::GetInstance()->GetCommandList();
        cmdList->SetPipelineState(verticalBlurSet_.pso.Get());
        cmdList->SetGraphicsRootSignature(verticalBlurSet_.rootSig.Get());
        cmdList->SetGraphicsRootConstantBufferView(0, blurConstantBuffer_->GetGPUVirtualAddress());
        uint32_t srvIndex = RTVManager::GetInstance()->GetRenderTexture(input)->GetSRVindexofRTV();
        cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));
        cmdList->DrawInstanced(3, 1, 0, 0);
    }

}

void Bloom::ApplyCombine(const std::string& input, const std::string& bloom, const std::string& output)
{
    RTVManager::GetInstance()->SetRenderTexture(output);
    auto cmdList = DXCommon::GetInstance()->GetCommandList();
    cmdList->SetPipelineState(combineSet_.pso.Get());
    cmdList->SetGraphicsRootSignature(combineSet_.rootSig.Get());
    cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
    uint32_t srvIndex = RTVManager::GetInstance()->GetRenderTexture(input)->GetSRVindexofRTV();
    cmdList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex));
    uint32_t bloomSrvIndex = RTVManager::GetInstance()->GetRenderTexture(bloom)->GetSRVindexofRTV();
    cmdList->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(bloomSrvIndex));
    cmdList->DrawInstanced(3, 1, 0, 0);
}

