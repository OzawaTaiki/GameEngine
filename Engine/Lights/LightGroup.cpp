#include "LightGroup.h"
#include "DXCommon.h"


void LightGroup::Initialize()
{
    commandList_ = DXCommon::GetInstance()->GetCommandList();

    directoinalLight_ = std::make_unique<DirectionalLight>();
    pointLight_ = std::make_unique<PointLight>();
    spotLight_ = std::make_unique<SpotLight>();

    directoinalLight_->Initialize();
    pointLight_->Initialize();
    spotLight_->Initialize();
}

void LightGroup::Update()
{
}

void LightGroup::Draw()
{
}

void LightGroup::TransferData() const
{
    directoinalLight_->TransferData();
    pointLight_->TransferData();
    spotLight_->TransferData();

    // Dライト
    commandList_->SetGraphicsRootConstantBufferView(5, directoinalLight_->GetResource()->GetGPUVirtualAddress());
    // Pライト
    commandList_->SetGraphicsRootConstantBufferView(6, pointLight_->GetResource()->GetGPUVirtualAddress());
    // Ｓライト
    commandList_->SetGraphicsRootConstantBufferView(7, spotLight_->GetResource()->GetGPUVirtualAddress());
}
