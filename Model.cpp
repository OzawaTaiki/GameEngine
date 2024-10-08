#include "Model.h"
#include "ModelManager.h"
#include "DXCommon.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const std::string Model::defaultDirpath_ = "Resources/obj/";

void Model::Initialize()
{

}

void Model::Draw()
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView());
    commandList->IASetIndexBuffer(&mesh_->GetIndexBufferView());

    commandList->SetGraphicsRootConstantBufferView(0, material_->GetResource()->GetGPUVirtualAddress());
    // Dライト
    // カメラ（ｖｐ
    // Pライト
    // Ｓライト
    // カラー
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
