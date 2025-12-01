#include "PSOBuilder.h"
#include "PSOManager.h"
#include "PSOFactory.h"
#include <Core/DXCommon/ShaderCompiler/ShaderCompiler.h>

PSOBuilder PSOBuilder::Create()
{
    PSOBuilder builder;
    builder.ApplyDefaults();
    return builder;
}

void PSOBuilder::ApplyDefaults()
{
    // デフォルト値を設定
    ZeroMemory(&desc_, sizeof(desc_));

    // デフォルトのBlend
    desc_.BlendState = PSOFactory::CreateBlendDesc(PSOFlags::BlendMode::Normal);

    // デフォルトのRasterizer
    desc_.RasterizerState = PSOFactory::CreateRasterizerDesc(PSOFlags::CullMode::Back);

    // デフォルトのDepthStencil
    desc_.DepthStencilState = PSOFactory::CreateDepthStencilDesc(
        PSOFlags::DepthMode::Comb_mAll_fLessEqual
    );

    // デフォルトのフォーマット
    desc_.NumRenderTargets = 1;
    desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc_.SampleDesc.Count = 1;
    desc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
}

PSOBuilder& PSOBuilder::SetShaders(const std::string& _vsName, const std::string& _psName)
{
    SetVertexShader(_vsName);
    SetPixelShader(_psName);
    return *this;
}

PSOBuilder& PSOBuilder::SetVertexShader(const std::string& _name)
{
    vsBlob_ = ShaderCompiler::GetInstance()->Get(_name);
    desc_.VS = { vsBlob_->GetBufferPointer(), vsBlob_->GetBufferSize() };
    hasVertexShader_ = true;
    return *this;
}

PSOBuilder& PSOBuilder::SetPixelShader(const std::string& _name)
{
    psBlob_ = ShaderCompiler::GetInstance()->Get(_name);
    desc_.PS = { psBlob_->GetBufferPointer(), psBlob_->GetBufferSize() };
    hasPixelShader_ = true;
    return *this;
}

PSOBuilder& PSOBuilder::SetFlags(PSOFlags _flags)
{
    desc_.BlendState = PSOFactory::CreateBlendDesc(_flags.GetBlendMode());
    desc_.RasterizerState = PSOFactory::CreateRasterizerDesc(_flags.GetCullMode());
    desc_.DepthStencilState = PSOFactory::CreateDepthStencilDesc(_flags.GetDepthMode());
    return *this;
}

PSOBuilder& PSOBuilder::SetCullMode(PSOFlags::CullMode _mode)
{
    desc_.RasterizerState = PSOFactory::CreateRasterizerDesc(_mode);
    return *this;
}

PSOBuilder& PSOBuilder::SetDepthMode(PSOFlags::DepthMode _mode)
{
    desc_.DepthStencilState = PSOFactory::CreateDepthStencilDesc(_mode);
    return *this;
}

PSOBuilder& PSOBuilder::SetBlendMode(PSOFlags::BlendMode _mode)
{
    desc_.BlendState = PSOFactory::CreateBlendDesc(_mode);
    return *this;
}

PSOBuilder& PSOBuilder::SetDepthWrite(bool _enable)
{
    desc_.DepthStencilState.DepthWriteMask = _enable ?
        D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    return *this;
}

PSOBuilder& PSOBuilder::SetDepthTest(bool _enable)
{
    desc_.DepthStencilState.DepthEnable = _enable;
    return *this;
}

PSOBuilder& PSOBuilder::SetWireframe(bool _enable)
{
    desc_.RasterizerState.FillMode = _enable ?
        D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    return *this;
}

PSOBuilder& PSOBuilder::SetRootSignature(ID3D12RootSignature* _rootSig)
{
    rootSignature_ = _rootSig;
    desc_.pRootSignature = rootSignature_;
    hasRootSignature_ = true;
    return *this;
}

PSOBuilder& PSOBuilder::UseModelRootSignature()
{
    auto rootSig = PSOManager::GetInstance()->GetRootSignature(
        PSOFlags::Type::Model
    );
    assert(rootSig.has_value());
    rootSignature_ = rootSig.value();
    desc_.pRootSignature = rootSignature_;
    hasRootSignature_ = true;
    return *this;
}

PSOBuilder& PSOBuilder::UseModelInputLayout()
{
    static D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    desc_.InputLayout.pInputElementDescs = inputElements;
    desc_.InputLayout.NumElements = _countof(inputElements);
    return *this;
}

void PSOBuilder::Validate()
{
    assert(hasVertexShader_ && "Vertex shader not set");
    assert(hasPixelShader_ && "Pixel shader not set");
    assert(hasRootSignature_ && "Root signature not set");
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOBuilder::Build()
{
    Validate();
    return PSOFactory::GetInstance()->CreateGraphicsPSO(desc_);
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOBuilder::Build(ID3D12RootSignature* rootSignature)
{
    return Microsoft::WRL::ComPtr<ID3D12PipelineState>();
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOBuilder::BuildAndRegister(const std::string& _name)
{
    auto pso = Build();
    PSOManager::GetInstance()->RegisterPSO(_name, pso.Get());
    PSOManager::GetInstance()->RegisterRootSignature(_name, rootSignature_);
    return pso;
}