#include "Material.h"
#include "DXCommon.h"
#include "MatrixFunction.h"
#include "TextureManager.h"
void Material::Initialize(const std::string& _texturepath)
{
	DXCommon* dxCommon = DXCommon::GetInstance();

	resorces_ = dxCommon->CreateBufferResource(sizeof(DataForGPU));
	resorces_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

	transform_ = { 0.0f,0.0f };
	scale_ = { 1.0f,1.0f };
	rotation_ = 0.0f;

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
	Vector3 trans = { transform_,0.0f };
	Vector3 scale = { scale_,1.0f };
	Vector3 rotate = { 0.0f,0.0f ,rotation_ };
	Matrix4x4 affine = MakeAffineMatrix(scale, rotate, trans);

	constMap_->uvTransform = affine;
	constMap_->shininess = shiness_;
	constMap_->enabledLighthig = enableLighting_;
}

void Material::MateriallQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const
{
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
