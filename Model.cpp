#include "Model.h"
#include "ModelManager.h"
#include "DXCommon.h"
#include "TextureManager.h"
#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const std::string Model::defaultDirpath_ = "Resources/obj/";

void Model::Initialize()
{

}

void Model::Draw(uint32_t _textureHandle)
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, mesh_->GetVertexBufferView());
    commandList->IASetIndexBuffer(mesh_->GetIndexBufferView());

    // カメラ（ｖｐ
    //commandList->SetGraphicsRootConstantBufferView(0, );
    // トランスフォーム
    //commandList->SetGraphicsRootConstantBufferView(1, );
    // マテリアル
    commandList->SetGraphicsRootConstantBufferView(2, material_->GetResource()->GetGPUVirtualAddress());
    // カラー
    //commandList->SetGraphicsRootConstantBufferView(3, );
    // テクスチャ
    commandList->SetGraphicsRootDescriptorTable(4, TextureManager::GetInstance()->GetGPUHandle(_textureHandle));
    // Dライト
    //commandList->SetGraphicsRootConstantBufferView(5, );
    // Pライト
    //commandList->SetGraphicsRootConstantBufferView(6, );
    // Ｓライト
    //commandList->SetGraphicsRootConstantBufferView(7, );
}

Model* Model::CreateFromObj(const std::string& _filePath)
{
    Model* model = ModelManager::GetInstance()->FindSameModel(_filePath);

    if (model->mesh_ == nullptr || model->material_ == nullptr)
    {
        model->LoadMesh(_filePath);
        model->LoadMaterial(_filePath);
    }

    return model;
}

void Model::LoadMesh(const std::string& _filePath)
{
    name_ = _filePath;
    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize();
    mesh_->LoadFile(_filePath);
}

void Model::LoadMaterial(const std::string& _filePath)
{
    material_ = std::make_unique<Material>();
    material_->Initialize(mesh_->GetTexturePath());
    material_->LoadTexture();
}

void Model::TransferData()
{
}
