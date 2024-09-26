#include "Model.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


const std::string Model::defaultDirectory_ = "Resources/obj";

Model* Model::CreateFromObj(const std::string& _modelName, const std::string& _extension)
{

    Model* instance = new Model;
	instance->LoadModel(_modelName, _extension);

    return instance;
}

void Model::LoadModel(const std::string& _modelname, const std::string& _extension)
{
	mesh_.emplace_back(std::make_unique<Mesh>());

	std::string directoryPath = defaultDirectory_ + "/" + _modelname;

    Assimp::Importer importer;
	std::string filepath = directoryPath + "/" + _modelname + "./" + _extension;
    const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
    assert(scene->HasMeshes());// メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());						//法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0));			// TexcoordがないMeshは今回は非対応
		mesh_.emplace_back();
		//ここからMeshの中身(Face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);				//三角形のみサポート
			//ここからFaceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				Mesh::VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				//aiProcess_MakeLeftHandedはz *= -1で、右手->左手に変換するので手動で対処
				vertex.position.z *= -1.0f;
				vertex.normal.z *= -1.0f;
				mesh_.back()->AddVertex(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			std::string fileName = directoryPath + "/" + textureFilePath.C_Str();
			this->LoadMaterial(fileName);
		}

	}
}

void Model::LoadMaterial(const std::string& fileName)
{
	material_.emplace_back(std::make_unique<Material>());


}
