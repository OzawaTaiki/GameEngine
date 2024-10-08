#include "TextureManager.h"
#include "DXCommon.h"
#include "Debug.h"

#include <cassert>

const std::string TextureManager::defaultDirpath_ = "Resources/images/";

TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize()
{
    dxCommon_ = DXCommon::GetInstance();
    assert(dxCommon_);
}

void TextureManager::Update()
{

}

void TextureManager::LoadTexture(const std::string& _filepath)
{
	assert(dxCommon_ != nullptr && "not initialized");

	size_t index = textures_.size();

	std::string fullpath = defaultDirpath_ + _filepath;
	DirectX::ScratchImage mipImages = GetMipImage(fullpath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textures_[_filepath].resource =  CreateTextureResource(metadata);
	textures_[_filepath].intermediateResource = UploadTextureData(textures_[_filepath].resource.Get(), mipImages);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures_[_filepath].srvHandlerCPU =dxCommon_->GetCPUSRVDescriptorHandle((uint32_t)index + 3);
	textures_[_filepath].srvHandlerGPU =dxCommon_->GetGPUSRVDescriptorHandle((uint32_t)index + 3);
	dxCommon_->GetDevice()->CreateShaderResourceView(textures_[_filepath].resource.Get(), &srvDesc, textures_[_filepath].srvHandlerCPU);

}

void TextureManager::LoadTextureAtMaterial(const std::string& _filepath)
{
	assert(dxCommon_ != nullptr && "not initialized");

	size_t index = textures_.size();

	std::string fullpath = _filepath;
	DirectX::ScratchImage mipImages = GetMipImage(fullpath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textures_[_filepath].resource = CreateTextureResource(metadata);
	textures_[_filepath].intermediateResource = UploadTextureData(textures_[_filepath].resource.Get(), mipImages);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures_[_filepath].srvHandlerCPU = dxCommon_->GetCPUSRVDescriptorHandle((uint32_t)index + 3);
	textures_[_filepath].srvHandlerGPU = dxCommon_->GetGPUSRVDescriptorHandle((uint32_t)index + 3);
	dxCommon_->GetDevice()->CreateShaderResourceView(textures_[_filepath].resource.Get(), &srvDesc, textures_[_filepath].srvHandlerCPU);
}

DirectX::ScratchImage TextureManager::GetMipImage(const std::string& _filepath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathw = Debug::ConvertString(_filepath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの生成
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す	
	return mipImage;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& _metadata)
{
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(_metadata.width); // Textureの幅
	resourceDesc.Height = UINT(_metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(_metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(_metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = _metadata.format; // Textureのフォーマット 
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、通常は1
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(_metadata.dimension); // Textureの次元。省略はしているのは2次元


	// 利用するHeapの設定。非常に特殊な運用、02_04から一部的なケース版がある
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う

	//resourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,//heapの設定
		D3D12_HEAP_FLAG_NONE,//heapの特殊な設定。特になし
		&resourceDesc,//resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,//データ転送される設定
		nullptr,//clearの最適値 使わないのでnullptr
		IID_PPV_ARGS(resource.GetAddressOf()));//作成するresourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(ID3D12Resource* _texture, const DirectX::ScratchImage& _mipImages)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(dxCommon_->GetDevice(), _mipImages.GetImages(), _mipImages.GetImageCount(), _mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(_texture, 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->CreateBufferResource(static_cast<uint32_t>(intermediateSize));
	UpdateSubresources(dxCommon_->GetCommandList(), _texture, intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	//Tetureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = _texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
	return intermediateResource;
}