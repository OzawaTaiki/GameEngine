#include <Features/Sprite/SpriteManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <cassert>

SpriteManager* SpriteManager::GetInstance()
{
    static SpriteManager instance;
    return &instance;
}

void SpriteManager::Initialize()
{
    psoFlags_ = PSOFlags::Type_Sprite | PSOFlags::Blend_Normal | PSOFlags::Cull_Back;

    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature(psoFlags_);
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();
}

void SpriteManager::PreDraw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
}

Sprite* SpriteManager::Create(const std::string& _name)
{
    sprites_.push_back(std::make_unique<Sprite>(_name));
    return sprites_.back().get();
}
