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
    graphicsPipelineState_[0] = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Model");
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_[0] = rootSignature.value();

    /// PSOを取得
    pso = PSOManager::GetInstance()->GetPipeLineStateObject("AnimationModel", blendMode_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ [1] = pso.value();

    /// RootSingnatureを取得
    rootSignature = PSOManager::GetInstance()->GetRootSignature("AnimationModel");
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_[1] = rootSignature.value();

}

void ModelManager::PreDrawForObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_[0]);
    commandList->SetPipelineState(graphicsPipelineState_[0]);
}

void ModelManager::PreDrawForAnimationModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_[1]);
    commandList->SetPipelineState(graphicsPipelineState_[1]);
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
