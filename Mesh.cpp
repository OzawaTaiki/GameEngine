#include "Mesh.h"
#include "DXCommon.h"

#include <cassert>
#include <iterator>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Mesh::Initialize()
{
    dxCommon = DXCommon::GetInstance();
    vertices_.clear();
    indices_.clear();
}

void Mesh::LoadFile(const std::string& _filepath,  const std::string& _directoryPath)
{
    name_ = _filepath;

    Assimp::Importer importer;
    std::string filepath = _directoryPath + _filepath;
    const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
    assert(scene->HasMeshes());// メッシュがないのは対応しない

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());						// 法線がないMeshは今回は非対応
        assert(mesh->HasTextureCoords(0));				// TexcoordがないMeshは今回は非対応


        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形のみサポート
            for (uint32_t element = 0; element < face.mNumIndices; ++element) {
                uint32_t vertexIndex = face.mIndices[element];

                aiVector3D& position = mesh->mVertices[vertexIndex];
                aiVector3D& normal = mesh->mNormals[vertexIndex];
                aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

                VertexData vertex = {};
                vertex.position = { position.x, position.y, position.z, 1.0f };
                vertex.normal = { normal.x, normal.y, normal.z };
                vertex.texcoord = { texcoord.x, texcoord.y };

                vertex.position.z *= -1.0f;  // Z反転
                vertex.normal.z *= -1.0f;    // Z反転

                auto it = std::find(vertices_.begin(), vertices_.end(), vertex);

                if (it == vertices_.end()) {					
                    // 値が重複しないとき
                    vertices_.push_back(vertex);
                    indices_.push_back(static_cast<uint32_t>(vertices_.size() - 1));
                }
                else {
                    size_t index = std::distance(vertices_.begin(), it);
                    indices_.push_back(static_cast<uint32_t>(index));
                }

            }
        }
    }

    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
            textureHandlePath_ = _directoryPath + "/" + textureFilePath.C_Str();
        }
    }

    InitializeReources();
    TransferData();
}

void Mesh::TransferData()
{
    std::memcpy(vConstMap_, vertices_.data(), sizeof(VertexData) * vertices_.size());
    std::memcpy(iConstMap_, indices_.data(), sizeof(uint32_t) * indices_.size());

}

void Mesh::InitializeReources()
{
    CreateResources();
    Map();

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<UINT> (sizeof(uint32_t) * indices_.size());
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

}

void Mesh::CreateResources()
{
    vertexResource_ = dxCommon->CreateBufferResource(static_cast<uint32_t>(sizeof(VertexData) * vertices_.size()));
    indexResource_ = dxCommon->CreateBufferResource(static_cast<uint32_t>(sizeof(uint32_t) * indices_.size()));
}

void Mesh::Map()
{
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMap_));
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&iConstMap_));


};
