#include <Features/Model/Material/Material.h>
#include <Core/DXCommon/DXCommon.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <assimp/material.h>

void Material::Initialize(const std::string& _texturepath)
{
	DXCommon* dxCommon = DXCommon::GetInstance();

	resorces_ = dxCommon->CreateBufferResource(sizeof(DataForGPU));
	resorces_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

	uvTransform_.SetOffset(Vector2(0.0f, 0.0f));
	uvTransform_.SetScale(Vector2(1.0f, 1.0f));
	uvTransform_.SetRotation(0.0f);


	shiness_ = 40.0f;

	enableLighting_ = true;

	texturePath_ = _texturepath;

	TransferData();
    LoadTexture();

}
 
void Material::LoadTexture()
{
	if (texturePath_ == "")
		textureHandle_ = 0;
	else
		textureHandle_ = TextureManager::GetInstance()->Load(texturePath_, "");
}

void Material::TransferData()
{
	Matrix4x4 affine = uvTransform_.GetMatrix();

	constMap_->uvTransform = affine;
	constMap_->deffuseColor = deffuseColor_;
	constMap_->shininess = shiness_;
	constMap_->enabledLighthig = enableLighting_;
    constMap_->hasTexture = hasTexture_ ? 1 : 0;
    constMap_->envScale = envScale_;
    constMap_->enableEnvironment = enableEnvironment_ ? 1 : 0;
}

void Material::MaterialQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index)
{
	TransferData();
    _commandList->SetGraphicsRootConstantBufferView(_index, resorces_->GetGPUVirtualAddress());
}

void Material::TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const
{
    _commandList->SetGraphicsRootDescriptorTable(_index, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
}

void Material::TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index, uint32_t _textureHandle) const
{
	_commandList->SetGraphicsRootDescriptorTable(_index, TextureManager::GetInstance()->GetGPUHandle(_textureHandle));
}

void Material::AnalyzeMaterial(const aiMaterial* _material)
{
    aiUVTransform uvTransform;
	if (_material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), uvTransform) == AI_SUCCESS)
	{
        uvTransform_.SetOffset(Vector2(uvTransform.mTranslation.x, uvTransform.mTranslation.y));
        uvTransform_.SetScale(Vector2(uvTransform.mScaling.x, uvTransform.mScaling.y));
        uvTransform_.SetRotation(uvTransform.mRotation);
    }
    else
    {
        uvTransform_.SetOffset(Vector2(0.0f, 0.0f));
        uvTransform_.SetScale(Vector2(1.0f, 1.0f));
        uvTransform_.SetRotation(0.0f);
    }

	aiColor3D meshColor;
	if (_material->Get(AI_MATKEY_COLOR_DIFFUSE, meshColor) == AI_SUCCESS)
	{
		deffuseColor_ = Vector4(meshColor.r, meshColor.g, meshColor.b, 1.0f);
	}
    else
    {
        deffuseColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // デフォルトの色
    }
    if (_material->Get(AI_MATKEY_SHININESS, shiness_) != AI_SUCCESS)
    {
        shiness_ = 40.0f; // デフォルトのシニアス値
    }

	if (_material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		hasTexture_ = true;
	else
		hasTexture_ = false;
}

void Material::Imgui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    ImGui::ColorEdit4("Diffuse Color", &deffuseColor_.x);
    ImGui::DragFloat("Shininess", &shiness_, 0.1f, 0.0f, 100.0f);
    ImGui::Checkbox("Enable Lighting", &enableLighting_);
    ImGui::Checkbox("Enable Environment", &enableEnvironment_);
    ImGui::DragFloat("Environment Scale", &envScale_, 0.01f, 0.0f, 10.0f);

    ImGui::Text("Texture Path: %s", texturePath_.c_str());
    ImGui::SeparatorText("UV Transform");

    Vector2 offset = uvTransform_.GetOffset();
    Vector2 scale = uvTransform_.GetScale();
    float rotation = uvTransform_.GetRotation();

    ImGui::DragFloat2("UV Offset", &offset.x, 0.01f);
    ImGui::DragFloat2("UV Scale", &scale.x, 0.01f);
    ImGui::DragFloat("UV Rotation", &rotation, 0.01f);

    uvTransform_.SetOffset(offset);
    uvTransform_.SetScale(scale);
    uvTransform_.SetRotation(rotation);

    ImGui::PopID();
#endif // _DEBUG
}
