#include "SkyBox.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <Features/Camera/Camera/Camera.h>

void SkyBox::Initialize(const Vector3& _scale, bool _useQuaternion)
{
    // PSO
    PSOManager* psoManager = PSOManager::GetInstance();
    psoManager->CreatePSOForSkyBox();


    auto pso = psoManager->GetPipeLineStateObject(PSOFlags::Combine(PSOFlags::Type::SkyBox , PSOFlags::BlendMode::Normal , PSOFlags::CullMode::Back,PSOFlags::DepthMode::Comb_mAll_fLessEqual));
    if (!pso.has_value() || pso.value() == nullptr)
    {
        assert(0 && "PSO for SkyBox is not created");
        return;
    }
    pipelineState_ = pso.value();

    // RootSignature
    auto rs = psoManager->GetRootSignature(PSOFlags::Type::SkyBox);
    if (!rs.has_value() || rs.value() == nullptr)
    {
        assert(0 && "RootSignature for SkyBox is not created");
        return;
    }
    rootSignature_ = rs.value();


    // 初期化
    worldTransform_.Initialize();
    worldTransform_.scale_ = _scale;
    worldTransform_.UpdateData(_useQuaternion);

    useQuaternion_ = _useQuaternion;

    objectColor_.Initialize();

    isDirty_ = true;

}

void SkyBox::Draw(const Camera* _camera)
{
    UpdateWorldTransform();
    // 描画
    ID3D12GraphicsCommandList* cmdList = DXCommon::GetInstance()->GetCommandList();

    cmdList->SetPipelineState(pipelineState_);
    cmdList->SetGraphicsRootSignature(rootSignature_);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    worldTransform_.QueueCommand(cmdList, 0);
    _camera->QueueCommand(cmdList, 1);
    objectColor_.QueueCommand(cmdList, 2);

    cmdList->SetGraphicsRootDescriptorTable(3, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));


    cmdList->DrawInstanced(kIndexNum, 1, 0, 0);

}


void SkyBox::SetColor(const Vector4& _color)
{
    objectColor_.SetColor(_color);
}

void SkyBox::SetPosition(const Vector3& _position)
{
    worldTransform_.transform_ = _position;
    isDirty_ = true;
}

void SkyBox::SetScale(const Vector3& _scale)
{
    worldTransform_.scale_ = _scale;
    isDirty_ = true;
}

void SkyBox::SetRotation(const Vector3& _rotation)
{
    assert(useQuaternion_ == false);

    worldTransform_.rotate_ = _rotation;
    isDirty_ = true;
}

void SkyBox::SetRotation(const Quaternion& _rotation)
{
    assert(useQuaternion_ == true);

    worldTransform_.quaternion_ = _rotation;
    isDirty_ = true;
}

void SkyBox::SetTexture(const std::string& _name)
{
    // 拡張子を取得
    std::string ext = _name.substr(_name.find_last_of(".") + 1);
    // 拡張子がdssじゃない場合は、弾く
    if (ext != "dds")
    {
        assert(0 && "not dds");
        return;
    }

    textureHandle_ = TextureManager::GetInstance()->Load(_name);
}

void SkyBox::SetTexture(uint32_t _handle)
{
    textureHandle_ = _handle;
}

void SkyBox::QueueCmdCubeTexture(uint32_t _index) const
{
    auto cmdList = DXCommon::GetInstance()->GetCommandList();
    cmdList->SetGraphicsRootDescriptorTable(_index, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
}

void SkyBox::UpdateWorldTransform()
{
    if (isDirty_)
    {
        worldTransform_.UpdateData(useQuaternion_);
        isDirty_ = false;
    }
}
