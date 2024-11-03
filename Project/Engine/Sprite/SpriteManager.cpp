#include "SpriteManager.h"
#include "DXCommon.h"
#include <cassert>

SpriteManager* SpriteManager::GetInstance()
{
    static SpriteManager instance;
    return &instance;
}

void SpriteManager::Initialize()
{
    blendMode_ = PSOManager::BlendMode::Normal;

    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("Sprite", blendMode_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Sprite");
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();


}

void SpriteManager::PreDraw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
}

Sprite* SpriteManager::Create()
{
    sprites_.push_back(std::make_unique<Sprite>());
    return sprites_.back().get();
}
