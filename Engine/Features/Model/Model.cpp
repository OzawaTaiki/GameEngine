#include <Features/Model/Model.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Model/Color/ObjectColor.h>
#include <Features/Model/Transform/WorldTransform.h>

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Debug/Debug.h>

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const std::string Model::defaultDirpath_ = "Resources/models/";

void Model::Initialize()
{

}

void Model::Update(float _deltaTime)
{
    if (!currentAnimation_ || !currentAnimation_->IsPlaying())
    {
        return;
    }

    currentAnimation_->Update(skeleton_.GetJoints(), _deltaTime);

    // アニメーションが終わったらアニメーションを解除
    if (!currentAnimation_->IsPlaying())
    {
        preAnimation_ = currentAnimation_.get();
        return;
    }

    skeleton_.Update();
    skinCluster_.Update(skeleton_.GetJoints());

    if (skinningCS_)
        skinningCS_->Execute();
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
        material_[mesh->GetUseMaterialIndex()]->TransferData();
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(commandList, 2);
        // カラー
        _color->QueueCommand(commandList, 3);
        //commandList->SetGraphicsRootConstantBufferView(3, _color->GetResource()->GetGPUVirtualAddress());
        // テクスチャ
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(commandList, 4, _textureHandle);
        // ライトたち
        QueueLightCommand(commandList, 5);

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
        material_[mesh->GetUseMaterialIndex()]->TransferData();
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(commandList, 2);
        // カラー
        _color->QueueCommand(commandList, 3);
        //commandList->SetGraphicsRootConstantBufferView(3, _color->GetResource()->GetGPUVirtualAddress());
        // テクスチャ
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(commandList, 4);
        // ライトたち
        QueueLightCommand(commandList, 5);

        commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::DrawSkeleton(const Matrix4x4& _wMat)
{
    skeleton_.Draw(_wMat);
}


Model* Model::CreateFromFile(const std::string& _filePath)
{
    Model* model = ModelManager::GetInstance()->FindSameModel(_filePath);

    if (model == nullptr)
    {
        model = ModelManager::GetInstance()->Create(_filePath);
        model->LoadFile(_filePath);
    }

    model->lightGroup_ = std::make_unique<LightGroup>();
    model->lightGroup_->Initialize();

    return model;
}

Model* Model::CreateFromMesh(std::unique_ptr<Mesh> _mesh)
{
    Model* model = ModelManager::GetInstance()->Create("MeshGene");

    model->mesh_.push_back(std::move(_mesh));
    model->material_.push_back(std::make_unique<Material>());
    model->material_[0]->Initialize("");


    model->lightGroup_ = std::make_unique<LightGroup>();
    model->lightGroup_->Initialize();

    return model;
}

Model* Model::CreateFromVertices(std::vector<VertexData> _vertices, std::vector<uint32_t> _indices, const std::string& _name)
{
    Model* model = ModelManager::GetInstance()->Create(_name);

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->Initialize(_vertices, _indices, _name);
    model->mesh_.push_back(std::move(mesh));

    std::unique_ptr<Material> material = std::make_unique<Material>();
    material->Initialize("");
    model->material_.push_back(std::move(material));

    model->lightGroup_ = std::make_unique<LightGroup>();
    model->lightGroup_->Initialize();

    return model;
}


void Model::QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList) const
{
    QueueLightCommand(_commandList, 5);

    if(margedMesh_)
    {
        _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        margedMesh_->QueueCommand(_commandList);
        for (size_t meshIndex = 0; meshIndex < margedMesh_->GetMeshCount(); ++meshIndex)
        {
            UINT vertexOffset = margedMesh_->GetVertexOffset(meshIndex);
            UINT indexOffset = margedMesh_->GetIndexOffset(meshIndex);
            UINT indexCount = margedMesh_->GetIndexCount(meshIndex);

            material_[mesh_[meshIndex]->GetUseMaterialIndex()]->TransferData();
            material_[mesh_[meshIndex]->GetUseMaterialIndex()]->MaterialQueueCommand(_commandList, 2);
            material_[mesh_[meshIndex]->GetUseMaterialIndex()]->TextureQueueCommand(_commandList, 4);
            _commandList->DrawIndexedInstanced(indexCount, 1, indexOffset, vertexOffset, 0);
        }
    }
    else
    {
        for (auto& mesh : mesh_)
        {
            mesh->QueueCommand(_commandList);
            material_[mesh->GetUseMaterialIndex()]->TransferData();
            material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(_commandList, 2);
            material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(_commandList, 4);
            _commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
        }
    }

}

void Model::QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, uint32_t _textureHandle) const
{
    QueueLightCommand(_commandList, 5);


    for (auto& mesh : mesh_)
    {
        mesh->QueueCommand(_commandList);
        material_[mesh->GetUseMaterialIndex()]->TransferData();
        material_[mesh->GetUseMaterialIndex()]->MaterialQueueCommand(_commandList, 2);
        material_[mesh->GetUseMaterialIndex()]->TextureQueueCommand(_commandList, 4, _textureHandle);
        _commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }
}

void Model::QueueCommandForShadow(ID3D12GraphicsCommandList* _commandList) const
{
    QueueLightCommand(_commandList, 2);
    for (auto& mesh : mesh_)
    {
        mesh->QueueCommand(_commandList);
        _commandList->DrawIndexedInstanced(mesh->GetIndexNum(), 1, 0, 0, 0);
    }

}

void Model::QueueLightCommand(ID3D12GraphicsCommandList* _commandList,uint32_t _index) const
{
    LightingSystem::GetInstance()->QueueGraphicsCommand(_commandList, _index);
}

void Model::SetAnimation(const std::string& _name,bool _loop)
{
    if (!currentAnimation_)
    {
        return;
    }

    if (animation_.find(_name) == animation_.end())
    {
        assert(false && "アニメーションが見つかりません");
        return;
    }
    currentAnimation_->Reset();
    currentAnimation_->SetAnimation(animation_[_name]->GetAnimation());
    currentAnimation_->SetLoop(_loop);

}

void Model::ChangeAnimation(const std::string& _name,float _blendTime, bool _loop)
{
    if (!currentAnimation_)
    {
        return;
    }

    if (animation_.find(_name) == animation_.end())
    {
        assert(false && "アニメーションが見つかりません");
        return;
    }
    currentAnimation_->Reset();
    currentAnimation_->ChangeAnimation(animation_[_name]->GetAnimation(), _blendTime);
    //currentAnimation_ = animation_[_name].get();
    currentAnimation_->SetLoop(_loop);
}

void Model::ToIdle(float _timeToIdle)
{
    if (currentAnimation_)
    {
        currentAnimation_->ToIdle(_timeToIdle);
    }
    else if(preAnimation_)
    {
        //currentAnimation_ = preAnimation_;
        currentAnimation_->ToIdle(_timeToIdle);
    }
}

void Model::LoadAnimation(const std::string& _filePath, const std::string& _name)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(defaultDirpath_ + _filePath, aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    assert(scene->HasAnimations());
    //LoadAnimation(scene, defaultDirpath_ + _filePath, _name);

    LoadAnimation(scene, defaultDirpath_ + _filePath, _name);
}

ID3D12Resource* Model::GetIndexResource(size_t _index)
{
    if (_index == -1)
        return mesh_[0]->GetIndexResource();
    else
        return mesh_[_index]->GetIndexResource();
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

bool Model::HasAnimation() const
{
    return currentAnimation_ != nullptr && !animation_.empty();
}

bool Model::IsAnimationPlaying() const
{
    return currentAnimation_ && currentAnimation_->IsPlaying();
}

void Model::LoadFile(const std::string& _filepath)
{
#ifdef _DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif // _DEBUG

    Debug::Log("loading : filepath:" + defaultDirpath_ + _filepath + "\n");
    name_ = _filepath;

    Assimp::Importer importer;
    std::string filepath = defaultDirpath_ + _filepath;
    const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->HasMeshes()) {
        Debug::Log("Failed to load model file: " + filepath + "\n");
        Debug::Log("\tERROR::ASSIMP::" + std::string(importer.GetErrorString()) + "\n");
        throw std::runtime_error("Failed to load model file");
        return;
    }

    LoadMesh(scene);
    LoadMaterial(scene);

    if (scene->HasAnimations())
    {
        LoadAnimation(scene, filepath,"");
    }

#ifdef _DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::string str = std::to_string(duration) + " ms\n";
    Debug::Log("Load complete " + str);
#else
    Debug::Log("Load complete\n");
#endif // _DEBUG
    //Debug::Log("data\nvertex :" + std::to_string(vertices_.size()) + "\nindex :" + std::to_string(indices_.size()) + "\n");

}

void Model::LoadMesh(const aiScene* _scene)
{
    // メッシュの読み込み

    uint32_t currentVertexOffset = 0;

    for (uint32_t meshIndex = 0; meshIndex < _scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = _scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());						    // 法線がないMeshは今回は非対応
        bool hasTexCoords = mesh->HasTextureCoords(0); // テクスチャ座標があるかどうか
        std::unique_ptr<Mesh> pMesh = std::make_unique<Mesh>();
        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;

        Vector3 min = { 16536 };
        Vector3 max = { -16536 };
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            VertexData vertex = {};
            vertex.position = { -mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z, 1.0f };
            vertex.normal = { -mesh->mNormals[vertexIndex].x, mesh->mNormals[vertexIndex].y, mesh->mNormals[vertexIndex].z };
            if (hasTexCoords)
            {
                vertex.texcoord = { mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y };
            }
            else
                vertex.texcoord = { 0.0f, 0.0f }; // テクスチャ座標がない場合はデフォルト値

            vertices.push_back(vertex);

            min = Vector3::Min(min, vertex.position.xyz());
            max = Vector3::Max(max, vertex.position.xyz());
        }


        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形のみサポート
            for (uint32_t index = 0; index < face.mNumIndices; ++index)
            {
                indices.push_back(face.mIndices[index]);
            }
        }

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            skinCluster_.CreateSkinCluster(mesh->mBones[boneIndex], currentVertexOffset);
        }

        currentVertexOffset += mesh->mNumVertices;

        pMesh->Initialize(vertices, indices,mesh->mName.C_Str());
        pMesh->SetMin(min);
        pMesh->SetMax(max);
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

        material_[materialIndex]->AnalyzeMaterial(material);

        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

            // TODO Materialからテクスチャを読み込む際のパスの設定を改善する

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

void Model::LoadAnimation(const aiScene* _scene, const std::string& _filepath, const std::string& _name)
{
    if (_scene->mNumAnimations == 0)
        return;

    //todo コンストラクタで初期化
    // 基本スキンクラスターでもってるからそこに持たせたほうがいいのかも？

    for (uint32_t animationIndex = 0; animationIndex < _scene->mNumAnimations; ++animationIndex)
    {
        std::string name = _name;
        // 引数が空ならシーンのアニメーション名を使う
        if (name.empty())
            name = _scene->mAnimations[animationIndex]->mName.C_Str();

        // シーンのアニメーション名が空なら、デフォルトの名前を生成
        if (name.empty())
        {
            name = "animation" + std::to_string(animationIndex + animation_.size());
        }

        animation_[name] = std::make_unique<ModelAnimation>();
        animation_[name]->ReadSampler(_filepath);
        animation_[name]->ReadAnimation(_scene->mAnimations[animationIndex]);
    }

    if (margedMesh_) // 既に生成済みなら
        return;

    LoadNode(_scene);
    CreateSkeleton();

    margedMesh_ = std::make_unique<MargedMesh>();
    margedMesh_->Initialize(mesh_);

    currentAnimation_ = std::make_unique<ModelAnimation>();
    currentAnimation_->Initialize();

    uint32_t vertexCount = static_cast<uint32_t>(margedMesh_->GetVertexCount());
    skinCluster_.CreateResources(static_cast<uint32_t>(skeleton_.GetJoints().size()), vertexCount, skeleton_.GetJointMap());


    margedMesh_->SetSkinnedVertexBufferView(SkinningCS::CreateOutputVertexResource(vertexCount));

    skinningCS_ = std::make_unique<SkinningCS>();
    skinningCS_->CreateSRVForInputVertexResource(margedMesh_->GetVertexResource(), vertexCount);
    skinningCS_->CreateSRVForInfluenceResource(skinCluster_.GetInfluenceResource(), vertexCount);
    skinningCS_->CreateSRVForOutputVertexResource(margedMesh_->GetSkinnedVertexResource(), vertexCount);
    skinningCS_->CreateSRVForMatrixPaletteResource(skinCluster_.GetPaletteResource(), static_cast<uint32_t>(skeleton_.GetJoints().size()));


    currentAnimation_->Update(skeleton_.GetJoints(), 0);

    skeleton_.Update();
    skinCluster_.Update(skeleton_.GetJoints());

    if (skinningCS_)
        skinningCS_->Execute();

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
