#include "Model.h"
#include "ModelManager.h"
#include "ObjectColor.h"
#include "WorldTransform.h"

#include <Core/DirectX/DXCommon.h>
#include <ResourceManagement/TextureManager/TextureManager.h>
#include <Framework/Camera/Camera.h>
#include <Physics/Math/MatrixFunction.h>
#include <Systems/Utility/Debug.h>

#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const std::string Model::defaultDirpath_ = "Resources/models/";

void Model::Initialize()
{

}

void Model::Update()
{
    if (currentAnimation_)
    {
        currentAnimation_->Update(skeleton_.GetJoints());
        // アニメーションが終わったらアニメーションを解除
        if (!currentAnimation_->IsPlaying())
            currentAnimation_ = nullptr;
    }
    skeleton_.Update();
    skinCluster_.Update(skeleton_.GetJoints());

}

void Model::Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, ObjectColor* _color)
{

    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    for (auto& mesh : mesh_)
    {
        mesh->QueueCommand(commandList);

        // カメラ（ｖｐ
        commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
        // トランスフォーム
        commandList->SetGraphicsRootConstantBufferView(1, _transform.GetResource()->GetGPUVirtualAddress());
        // マテリアル
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(commandList, 2);
        // カラー
        _color->QueueCommand(commandList, 3);
        //commandList->SetGraphicsRootConstantBufferView(3, _color->GetResource()->GetGPUVirtualAddress());
        // テクスチャ
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(commandList, 4, _textureHandle);
        // ライトたち
        lightGroup_->TransferData();

        commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::Draw(const WorldTransform& _transform, const Camera* _camera, ObjectColor* _color)
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();


    for (auto& mesh : mesh_)
    {
        mesh->QueueCommand(commandList);

        // カメラ（ｖｐ
        commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
        // トランスフォーム
        commandList->SetGraphicsRootConstantBufferView(1, _transform.GetResource()->GetGPUVirtualAddress());
        // マテリアル
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(commandList, 2);
        // カラー
        _color->QueueCommand(commandList, 3);
        //commandList->SetGraphicsRootConstantBufferView(3, _color->GetResource()->GetGPUVirtualAddress());
        // テクスチャ
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(commandList, 4);
        // ライトたち
        lightGroup_->TransferData();
        lightGroup_->QueueCommand(commandList);

        commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::DrawSkeleton(const Matrix4x4& _wMat)
{
    skeleton_.Draw(_wMat);
}

void Model::ShowImGui(const std::string& _name)
{

}

Model* Model::CreateFromObj(const std::string& _filePath)
{
    Model* model = ModelManager::GetInstance()->FindSameModel(_filePath);

    if (model->mesh_.size() == 0|| model->material_.size() == 0)
    {
       model-> LoadFile(_filePath);
    }

    model->lightGroup_ = std::make_unique<LightGroup>();
    model->lightGroup_->Initialize();
    return model;
}

void Model::QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, bool _animation) const
{
    lightGroup_->TransferData();
    lightGroup_->QueueCommand(_commandList);

    for (auto& mesh : mesh_)
    {
        if (!_animation)
            mesh->QueueCommand(_commandList);
        else
        {
            mesh->QueueCommand(_commandList, skinCluster_.GetInfluenceBufferView());
            skinCluster_.QueueCommand(_commandList);
        }
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(_commandList, 2);
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(_commandList, 4);
        _commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, uint32_t _textureHandle, bool _animation) const
{
    lightGroup_->TransferData();
    lightGroup_->QueueCommand(_commandList);

    for (auto& mesh : mesh_)
    {
        mesh->QueueCommand(_commandList);
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(_commandList, 2);
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(_commandList, 4, _textureHandle);
        _commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::SetAnimation(const std::string& _name,bool _loop)
{
    if (animation_.find(_name) == animation_.end())
    {
        assert(false && "アニメーションが見つかりません");
        return;
    }
    currentAnimation_ = animation_[_name].get();
    currentAnimation_->SetLoop(_loop);
}

void Model::ToIdle(float _timeToIdle)
{
    if (currentAnimation_)
    {
        currentAnimation_->ToIdle(_timeToIdle);
    }
}

void Model::SetUVTransform(const Vector2& _transform, uint32_t _index)
{
    material_[_index]->transform_ = _transform;
    material_[_index]->TransferData();
}

void Model::SetUVScale(const Vector2& _scale, uint32_t _index)
{
    material_[_index]->scale_ = _scale;
    material_[_index]->TransferData();
}

void Model::SetUVRotation(float _rotation, uint32_t _index)
{
    material_[_index]->rotation_ = _rotation;
    material_[_index]->TransferData();
}

Vector3 Model::GetMin(size_t _index) const
{
    if (_index == -1)
    {
        Vector3 min = { 16536,16536,16536 };
        for (auto& mesh : mesh_)
        {
            min = Vector3::Min(min, mesh->GetMin());
        }
        return min;
    }

    else
        return mesh_[_index]->GetMin();
}
Vector3 Model::GetMax(size_t _index) const
{
    if (_index == -1)
    {
        Vector3 max = { -16536,-16536,-16536 };
        for (auto& mesh : mesh_)
        {
            max = Vector3::Max(max, mesh->GetMax());
        }
        return max;
    }
    else
        return mesh_[_index]->GetMax();

}

Matrix4x4 Model::GetAnimationMatrix() const
{
    if (!currentAnimation_)
        return MakeIdentity4x4();
    // TODO : SetAnimetion関数作成 名前を引数で受け取りそのアニメーションをcurrentAnimetionにセット ↓とupdateでそれを使う
    return currentAnimation_->GetLocalMatrix();


}

void Model::LoadFile(const std::string& _filepath)
{
    auto start = std::chrono::high_resolution_clock::now();
    Utils::Log("load start\nfilepath:" + defaultDirpath_ + _filepath + "\n");
    name_ = _filepath;

    Assimp::Importer importer;
    std::string filepath = defaultDirpath_ + _filepath;
    const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate| aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
    assert(scene->HasMeshes());// メッシュがないのは対応しない

    LoadMesh(scene);
    LoadMaterial(scene);
    LoadAnimation(scene);
    LoadNode(scene);
    CreateSkeleton();

    skinCluster_.CreateResources(static_cast<uint32_t>(skeleton_.GetJoints().size()), mesh_[0]->GetVertexNum(), skeleton_.GetJointMap());


    TransferData();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::string str = std::to_string(duration);
    Utils::Log("Load finish \ntime :" + str + "ms\n");
    //Utils::Log("data\nvertex :" + std::to_string(vertices_.size()) + "\nindex :" + std::to_string(indices_.size()) + "\n");

}

void Model::LoadMesh(const aiScene* _scene)
{
    // メッシュの読み込み
    for (uint32_t meshIndex = 0; meshIndex < _scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = _scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());						    // 法線がないMeshは今回は非対応
        assert(mesh->HasTextureCoords(0));				    // TexcoordがないMeshは今回は非対応
        std::unique_ptr<Mesh> pMesh = std::make_unique<Mesh>();
        pMesh->Initialize();

        Vector3 min = { 16536 };
        Vector3 max = { -16536 };
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            Mesh::VertexData vertex = {};
            vertex.position = { -mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z, 1.0f };
            vertex.normal = { -mesh->mNormals[vertexIndex].x, mesh->mNormals[vertexIndex].y, mesh->mNormals[vertexIndex].z };
            vertex.texcoord = { mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y };

            pMesh->vertices_.push_back(vertex);

            min = Vector3::Min(min, vertex.position.xyz());
            max = Vector3::Max(max, vertex.position.xyz());
        }

        pMesh->SetMin(min);
        pMesh->SetMax(max);

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形のみサポート
            for (uint32_t index = 0; index < face.mNumIndices; ++index)
            {
                pMesh->indices_.push_back(face.mIndices[index]);
            }
        }

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            skinCluster_.CreateSkinCluster(mesh->mBones[boneIndex]);
        }

        pMesh->SetUseMaterialIndex(mesh->mMaterialIndex);
        pMesh->TransferData();


        mesh_.push_back(std::move(pMesh));
    }
}

void Model::LoadMaterial(const aiScene* _scene)
{
    material_.resize(_scene->mNumMaterials);
    for (uint32_t materialIndex = 0; materialIndex < _scene->mNumMaterials; ++materialIndex) {
        aiMaterial* material = _scene->mMaterials[materialIndex];

        std::string path = "";
        material_[materialIndex] = std::make_unique<Material>();

        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

            // /の位置を探す
            size_t slashPos = name_.find('/');

            if (slashPos != std::string::npos)
            {// 見つかったら
                std::string dirPath = name_.substr(0, slashPos);
                textureFilePath = dirPath + '/' + textureFilePath.C_Str();
            }

            path = defaultDirpath_ + "/" + textureFilePath.C_Str();
        }
        else if (path == "")
        {
            path = "Resources/images/uvChecker.png";
        }
        material_[materialIndex]->Initialize(path);
    }
}

void Model::LoadAnimation(const aiScene* _scene)
{
    if (_scene->mNumAnimations == 0)
        return;

    for (uint32_t animationIndex = 0; animationIndex < _scene->mNumAnimations; ++animationIndex)
    {
        std::string name = _scene->mAnimations[animationIndex]->mName.C_Str();
        animation_[name] = std::make_unique<ModelAnimation>();
        animation_[name]->ReadAnimation(_scene->mAnimations[animationIndex]);
    }
}

void Model::LoadNode(const aiScene* _scene)
{
    assert(_scene->mRootNode != nullptr);

    node_.ReadNode(_scene->mRootNode);
}

void Model::CreateSkeleton()
{
    skeleton_.CreateSkeleton(node_);
}

void Model::CreateSkinCluster(const aiMesh* _mesh)
{
    for (uint32_t boneIndex = 0; boneIndex < _mesh->mNumBones; ++boneIndex)
    {
        aiBone* bone = _mesh->mBones[boneIndex];
        std::string boneName = bone->mName.C_Str();

        for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
        {
        }
    }
}


void Model::TransferData()
{
}
