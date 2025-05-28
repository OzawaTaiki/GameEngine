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
    psoFlags_ = PSOFlags::Type_Model | PSOFlags::Blend_Normal | PSOFlags::Cull_Back | PSOFlags::Depth_mAll_fLEqual;
    psoFlagsForAlpha_ = PSOFlags::Type_Model | PSOFlags::Blend_Normal | PSOFlags::Cull_Back | PSOFlags::Depth_mZero_fLEqual;


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

    auto psoAlpha = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlagsForAlpha_);
    // PSOが生成されているか確認
    assert(psoAlpha.has_value() && psoAlpha != nullptr);
    graphicsPipelineStateForAlpha_ = psoAlpha.value();


 }

void ModelManager::PreDrawForObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ModelManager::PreDrawForAlphaObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineStateForAlpha_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

Model* ModelManager::FindSameModel(const std::string& _name)
{
    auto it = models_.find(_name);
    if (it != models_.end())
        return models_[_name].get();

    return nullptr;
}

Model* ModelManager::Create(const std::string& _name)
{
    std::string name = _name;

    auto it = models_.find(name);
    if (it != models_.end())
    {
        name = name + std::to_string(models_.size());
    }

    models_[name] = std::make_unique<Model>();
    return models_[name].get();
}
