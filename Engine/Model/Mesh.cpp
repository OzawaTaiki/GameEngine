#define NOMINMAX
#include "Mesh.h"
#include "DXCommon.h"
#include "Debug.h"
#include <cassert>
#include <iterator>
#include <chrono>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

template <>
struct std::hash<Mesh::VertexData> {
    size_t operator()(const Mesh::VertexData& v) const {
        size_t h1 = std::hash<Vector4>{}(v.position);
        size_t h2 = std::hash<Vector2>{}(v.texcoord);
        size_t h3 = std::hash<Vector3>{}(v.normal);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
};

void Mesh::Initialize()
{
    dxCommon = DXCommon::GetInstance();
    vertices_.clear();
    indices_.clear();
}

void Mesh::LoadFile(const std::string& _filepath,  const std::string& _directoryPath)
{
    auto start= std::chrono::high_resolution_clock::now();
    Utils::Log("load start\nfilepath:" + _directoryPath + _filepath + "\n");
    name_ = _filepath;

    Assimp::Importer importer;
    std::string filepath = _directoryPath + _filepath;
    const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
    assert(scene->HasMeshes());// メッシュがないのは対応しない

    std::unordered_map<VertexData, uint32_t> vertexMap = {};

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());						    // 法線がないMeshは今回は非対応
        assert(mesh->HasTextureCoords(0));				    // TexcoordがないMeshは今回は非対応

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

                auto it = vertexMap.find(vertex);
                if (it == vertexMap.end()) {
                    // 値が重複しないとき
                    uint32_t index = static_cast<uint32_t>(vertices_.size());
                    vertices_.push_back(vertex);
                    indices_.push_back(index);
                    vertexMap[vertex] = index;

                    min.x = std::min(min.x, vertex.position.x);
                    min.y = std::min(min.y, vertex.position.y);
                    min.z = std::min(min.z, vertex.position.z);
                    max.x = std::max(max.x, vertex.position.x);
                    max.y = std::max(max.y, vertex.position.y);
                    max.z = std::max(max.z, vertex.position.z);
                }
                else {
                    indices_.push_back(it->second);
                }
            }
        }
    }

    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

            // /の位置を探す
            size_t slashPos = _filepath.find('/');

            if (slashPos != std::string::npos)
            {// 見つかったら
                std::string dirPath = _filepath.substr(0, slashPos);
                textureFilePath = dirPath+'/' + textureFilePath.C_Str();
            }

            textureHandlePath_ = _directoryPath + "/" + textureFilePath.C_Str();
        }
        else if(textureHandlePath_=="")
        {
            textureHandlePath_ = "Resources/images/uvChecker.png";
        }
    }

    InitializeReources();
    TransferData();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::string str = std::to_string(duration);
    Utils::Log("Load finish \ntime :" + str + "ms\n");
    Utils::Log("data\nvertex :" + std::to_string(vertices_.size()) + "\nindex :" + std::to_string(indices_.size())+"\n");
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
