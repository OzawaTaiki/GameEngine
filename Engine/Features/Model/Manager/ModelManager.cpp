#include <Features/Model/Manager/ModelManager.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <cassert>

ModelManager* ModelManager::GetInstance()
{
    static ModelManager instance;
    return &instance;
}

void ModelManager::Initialize()
{
    psoFlags_ = PSOFlags::Type_Model | PSOFlags::Blend_Normal | PSOFlags::Cull_Back;


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

void ModelManager::PreDrawForObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

Model* ModelManager::GetModelPtr()
{
    std::string name = "hmm_";

    size_t index = models_.size();

    name += std::to_string(index);

    return FindSameModel(name);

}

Model* ModelManager::Create(const std::string& _name)
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
