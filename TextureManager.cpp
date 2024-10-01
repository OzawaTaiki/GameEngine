#define NOMINMAX
#include "TextureManager.h"

#include "Debug.h"
#include "DXCommon.h"

#include "externals/DirectXTex/d3dx12.h"
#include <cassert>

TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initilize(ID3D12Device* _device, const std::string& _directoryPath)
{
	HRESULT hresult = S_FALSE;

    device_ = _device;
    defaultDirectoryPath_ = _directoryPath;

	srvDescroptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC descRiptorHeapDesc{};
	descRiptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;    //シェーダ―リソースビュー(STV)用
	descRiptorHeapDesc.NumDescriptors = 128;                       //ダブルバッファ用に２つ。多くてもかまわない
	descRiptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hresult = device_->CreateDescriptorHeap(&descRiptorHeapDesc, IID_PPV_ARGS(&descriptorHeap_));
	assert(SUCCEEDED(hresult));

}

uint32_t TextureManager::Load(const std::string& _fileName, ID3D12GraphicsCommandList* _cl)
{
	TextureManager* instance = GetInstance();
	uint32_t index = static_cast<int32_t> (instance->FindTexture(_fileName));
	if (index != std::numeric_limits<uint32_t>::max()){
		return index;
	}

	instance->textures_.emplace_back();
	instance->textures_.back().name = _fileName;

	DirectX::ScratchImage mipImages = instance->LoadTexture(_fileName);
	instance->CreateSrv(mipImages, _cl);

	return static_cast<uint32_t> (instance->textures_.size() - 1);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHandle(uint32_t _textureHandle)
{
	assert(textures_.size() > _textureHandle);
	return textures_[_textureHandle].srvHandlerGPU;
}


uint32_t TextureManager::FindTexture(const std::string& _fileName)
{
	auto it = std::find_if(textures_.begin(), textures_.end(), [&](const auto& texture) {
		return texture.name == _fileName;
						   });

	if (it != textures_.end()){
		return static_cast<uint32_t>(std::distance(textures_.begin(), it));
	}


	// 見つからなかった場合、uint32_tの最大値を返す
	return std::numeric_limits<uint32_t>::max();
}


DirectX::ScratchImage TextureManager::LoadTexture(const std::string& _fileName)
{
	DirectX::ScratchImage image{};
	std::wstring filePathw = ConvertString(defaultDirectoryPath_ + _fileName);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの生成
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImage; 
}

void TextureManager::CreateSrv(const DirectX::ScratchImage& _mipImages, ID3D12GraphicsCommandList* _cl)
{
	const DirectX::TexMetadata& metadata = _mipImages.GetMetadata();

	uint32_t index = static_cast<uint32_t>(textures_.size() - 1);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures_[index].resource = CreateResource(metadata);
	textures_[index].intermediateResource = UploadData(textures_[index].resource, _mipImages,_cl);

	textures_[index].srvHandlerCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	textures_[index].srvHandlerCPU.ptr += (srvDescroptorSize_ * (index + 3));
	textures_[index].srvHandlerGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	textures_[index].srvHandlerGPU.ptr += (srvDescroptorSize_ * (index + 3));

	device_->CreateShaderResourceView(textures_[index].resource.Get(), &srvDesc, textures_[index].srvHandlerCPU);
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateResource(const DirectX::TexMetadata& _metadata)
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
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,//heapの設定
		D3D12_HEAP_FLAG_NONE,//heapの特殊な設定。特になし
		&resourceDesc,//resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,//データ転送される設定
		nullptr,//clearの最適値 使わないのでnullptr
		IID_PPV_ARGS(resource.GetAddressOf()));//作成するresourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* _cl)
{
	//auto commandList = DXCommon::GetInstance()->GetCommandList();
	auto commandList = _cl;

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource( intermediateSize);
	UpdateSubresources(commandList, texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	//Tetureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateBufferResource( size_t _sizeInBytes)
{
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;               // UploadHeapを使う

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合は別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = _sizeInBytes;                  // リソースのサイズ。今回はVector4を3個分
	//バッファの場合はこれらを１にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	HRESULT hr = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
												  &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												  IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}
