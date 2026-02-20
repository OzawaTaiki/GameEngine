#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOBuilder.h>
#include <Core/DXCommon/PSOManager/PSOFactory.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/ShaderCompiler/ShaderCompiler.h>
#include <Debug/Debug.h>
#include <Utility/ConvertString/ConvertString.h>
#include <cassert>


namespace Engine
{

PSOManager* PSOManager::GetInstance()

{
    static PSOManager instance;
    return &instance;
}

void PSOManager::Initialize()
{
    dxCommon_ = DXCommon::GetInstance();
    assert(dxCommon_);

    CreateDefaultPSOs();
}

std::optional<ID3D12PipelineState*>
PSOManager::GetPipeLineStateObject(PSOFlags _flag)
{
    uint64_t index = static_cast<uint64_t>(_flag);
    auto it = graphicsPipelineStates_.find(index);
    if (it != graphicsPipelineStates_.end())
        return graphicsPipelineStates_[index].Get();

    PSOFlags::Type type = _flag.GetType();
    switch (type)
    {
        case PSOFlags::Type::Model:
            CreatePSOForModel(_flag);
            break;
        case PSOFlags::Type::Sprite:
            CreatePSOForSprite(_flag);
            break;
        case PSOFlags::Type::LineDrawer:
            CreatePSOForLineDrawer(_flag);
            break;
        case PSOFlags::Type::Particle:
            CreatePSOForParticle(_flag);
            break;
        default:
            return std::nullopt;
    }
    return graphicsPipelineStates_[index].Get();
}

std::optional<ID3D12RootSignature*>
PSOManager::GetRootSignature(PSOFlags _flag)
{
    uint64_t type = _flag.GetTypeValue();
    auto it = rootSignatures_.find(type);
    if (it != rootSignatures_.end())
        return rootSignatures_[type].Get();

    // 未登録なら作成を試みる（またはデフォルトを返す）
    return std::nullopt;
}

void PSOManager::SetPipeLineStateObject(PSOFlags _flag)
{
    auto pso = GetPipeLineStateObject(_flag);
    assert(pso.has_value() && "PSOが見つかりません");

    dxCommon_->GetCommandList()->SetPipelineState(pso.value());
}

void PSOManager::SetRootSignature(PSOFlags _flag)
{
    auto rs = GetRootSignature(_flag);
    assert(rs.has_value() && "RootSignatureが見つかりません");
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rs.value());
}

std::optional<ID3D12PipelineState*>
PSOManager::GetPSO(const std::string& name)
{
    auto it = registerPSO_.find(name);

    if (it == registerPSO_.end())
    {
        return std::nullopt;
    }

    return registerPSO_[name].Get();
}

void PSOManager::SetPSOForPostEffect(const std::string& _name)
{
    auto it = postEffectPipelineStates_.find(_name);
    assert(it != postEffectPipelineStates_.end() &&
           "PostEffectPSOが見つかりません");

    dxCommon_->GetCommandList()->SetPipelineState(
        postEffectPipelineStates_[_name].Get());
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(
        rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::OffScreen)].Get());
}

void PSOManager::SetRegisterPSO(const std::string& _name)
{
    auto it = registerPSO_.find(_name);
    assert(it != registerPSO_.end() && "PSOが見つかりません");

    dxCommon_->GetCommandList()->SetPipelineState(registerPSO_[_name].Get());
}

void PSOManager::SetRegisterRootSignature(const std::string& _name)
{
    auto it = regiterRootSignature_.find(_name);
    assert(it != regiterRootSignature_.end() && "RootSignatureが見つかりません");

    dxCommon_->GetCommandList()->SetGraphicsRootSignature(
        regiterRootSignature_[_name].Get());
}

Microsoft::WRL::ComPtr<IDxcBlob> PSOManager::ComplieShader(
    const std::wstring& _filePath, const wchar_t* _profile,
    const std::wstring& _entryFuncName, const std::wstring& _dirPath)
{
    return ShaderCompiler::GetInstance()->Compile(_filePath, _profile,
                                                  _entryFuncName, _dirPath);
}

void PSOManager::CreatePSOForPostEffect(const std::string& _name,
                                        const std::wstring& _psFileName,
                                        const std::wstring& _entryFuncName,
                                        const std::wstring& _dirPath)
{
    // 必要ならシェーダーを登録
    ShaderCompiler::GetInstance()->Register(_name + "_PS", _psFileName, L"ps_6_0",
                                            _entryFuncName, _dirPath);

    postEffectPipelineStates_[_name] =
        PSOBuilder::Create()
        .SetShaders("FullScreen_VS", _name + "_PS")
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::OffScreen)]
            .Get())
        .UseFullScreenInputLayout()
        .SetFlags(PSOFlags::Type::OffScreen | PSOFlags::BlendMode::Normal |
                  PSOFlags::CullMode::None | PSOFlags::DepthMode::Disable)
        .Build();
}

void PSOManager::RegisterPSO(const std::string& _name,
                             ID3D12PipelineState* _pso)
{
    registerPSO_[_name] = _pso;
}
void PSOManager::RegisterRootSignature(const std::string& _name,
                                       ID3D12RootSignature* _rs)
{
    regiterRootSignature_[_name] = _rs;
}

void PSOManager::CreateDefaultPSOs()
{
    auto factory = PSOFactory::GetInstance();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Model)] =
        factory->CreateModelRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Sprite)] =
        factory->CreateSpriteRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Particle)] =
        factory->CreateParticleRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::LineDrawer)] =
        factory->CreateSpriteRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::OffScreen)] =
        factory->CreateOffScreenRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::SkyBox)] =
        factory->CreateSkyBoxRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::DLShadowMap)] =
        factory->CreateShadowMapRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::PLShadowMap)] =
        factory->CreatePLShadowMapRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Text)] =
        factory->CreateTextRootSignature();
    rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Composite)] =
        factory->CreateOffScreenRootSignature();

    CreatePSOForModel(PSOFlags::ForNormalModel());
    CreatePSOForModel(PSOFlags::ForAlphaModel());
    // CreatePSOForSprite(PSOFlags::ForSprite());
    CreatePSOForLineDrawer(PSOFlags::ForLineDrawer());
    CreatePSOForLineDrawer(PSOFlags::ForLineDrawerAlways());
    CreatePSOForParticle(PSOFlags::ForAddBlendParticle());
    CreatePSOForText();
    CreatePSOForOffScreen();
    CreatePSOForComposite(PSOFlags::BlendMode::Normal);
    CreatePSOForDLShadowMap();
    CreatePSOForPLShadowMap();
    CreatePSOForSkyBox();
}

void PSOManager::CreatePSOForModel(PSOFlags _flags)
{
    graphicsPipelineStates_[_flags] =
        PSOBuilder::Create()
        .SetShaders("Model_VS", "Model_PS")
        .SetFlags(_flags)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Model)]
            .Get())
        .UseModelInputLayout()
        .Build();
}

void PSOManager::CreatePSOForSprite(PSOFlags _flags)
{
    graphicsPipelineStates_[_flags] =
        PSOBuilder::Create()
        .SetShaders("Sprite_VS", "Sprite_PS")
        .SetFlags(_flags)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Sprite)]
            .Get())
        .UseSpriteInputLayout()
        .Build();
}

void PSOManager::CreatePSOForLineDrawer(PSOFlags _flags)
{
    graphicsPipelineStates_[_flags] =
        PSOBuilder::Create()
        .SetShaders("Line_VS", "Line_PS")
        .SetFlags(_flags)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::LineDrawer)]
            .Get())
        .UseLineDrawerInputLayout()
        .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
        .Build();
}

void PSOManager::CreatePSOForParticle(PSOFlags _flags)
{
    graphicsPipelineStates_[_flags] =
        PSOBuilder::Create()
        .SetShaders("Particle_VS", "Particle_PS")
        .SetFlags(_flags)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Particle)]
            .Get())
        .UseParticleInputLayout()
        .Build();
}

void PSOManager::CreatePSOForOffScreen()
{
    PSOFlags flag = PSOFlags::Type::OffScreen | PSOFlags::CullMode::None |
        PSOFlags::BlendMode::Normal | PSOFlags::DepthMode::Disable;
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetShaders("FullScreen_VS", "FullScreen_PS")
        .SetFlags(flag)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::OffScreen)]
            .Get())
        .UseFullScreenInputLayout()
        .Build();
}

void PSOManager::CreatePSOForComposite(PSOFlags::BlendMode _blendMode)
{
    PSOFlags flag = PSOFlags::Type::Composite | _blendMode;
    if (graphicsPipelineStates_.contains(flag))
        return;
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetVertexShader("FullScreen_VS")
        .SetPixelShader("Composite_PS")
        .SetFlags(flag)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Composite)]
            .Get())
        .UseFullScreenInputLayout()
        .Build();
}

void PSOManager::CreatePSOForText()
{
    PSOFlags flag = PSOFlags::ForText();
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetShaders("Text3DRenderer.VS", "Text3DRenderer.PS")
        .SetFlags(flag)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::Text)]
            .Get())
        .UseTextInputLayout()
        .Build();
}

void PSOManager::CreatePSOForSkyBox()
{
    PSOFlags flag = PSOFlags::Type::SkyBox | PSOFlags::BlendMode::Normal |
        PSOFlags::CullMode::Back |
        PSOFlags::DepthMode::Comb_mAll_fLessEqual;
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetShaders("SkyBox_VS", "SkyBox_PS")
        .SetFlags(flag)
        .SetRootSignature(
            rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::SkyBox)]
            .Get())
        .UseModelInputLayout()
        .Build();
}

void PSOManager::CreatePSOForDLShadowMap()
{
    PSOFlags flag = PSOFlags::Type::DLShadowMap;
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetShaders("DLShadowMap_VS", "DLShadowMap_PS")
        .SetFlags(flag)
        .SetRootSignature(rootSignatures_[static_cast<uint64_t>(
            PSOFlags::Type::DLShadowMap)]
            .Get())
        .UseModelInputLayout()
        .Build();
}

void PSOManager::CreatePSOForPLShadowMap()
{
    PSOFlags flag = PSOFlags::Type::PLShadowMap;
    graphicsPipelineStates_[flag] =
        PSOBuilder::Create()
        .SetShaders("PLShadowMap_VS", "PLShadowMap_PS")
        .SetGeometryShader("PLShadowMap_GS")
        .SetFlags(flag)
        .SetRootSignature(rootSignatures_[static_cast<uint64_t>(
            PSOFlags::Type::PLShadowMap)]
            .Get())
        .UseModelInputLayout()
        .Build();
}

D3D12_BLEND_DESC PSOManager::GetBlendDesc(PSOFlags _flag)
{
    return PSOFactory::CreateBlendDesc(_flag.GetBlendMode());
}
D3D12_RASTERIZER_DESC PSOManager::GetRasterizerDesc(PSOFlags _flag)
{
    return PSOFactory::CreateRasterizerDesc(_flag.GetCullMode());
}
D3D12_DEPTH_STENCIL_DESC PSOManager::GetDepthStencilDesc(PSOFlags _flag)
{
    return PSOFactory::CreateDepthStencilDesc(_flag.GetDepthMode());
}

} // namespace Engine
