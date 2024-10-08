#include "ModelManager.h"

#include "PSOManager.h"
#include "DXCommon.h"
#include <cassert>

ModelManager* ModelManager::GetInstance()
{
    static ModelManager instance;
    return &instance;
}

void ModelManager::Initialize()
{
    blendMode_ = PSOManager::BlendMode::Normal;

    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("Model", blendMode_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Model");
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_= rootSignature.value();

}

void ModelManager::PreDraw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
}

Model* ModelManager::FindSameModel(const std::string& _name)
{
    auto it = models_.find(_name);
    if (it != models_.end())
        return models_[_name].get();
    else
    {
        models_[_name] = std::make_unique<Model>();
        return models_[_name].get();
    }
}
