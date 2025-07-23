#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Debug/Debug.h>
#include <Utility/ConvertString/ConvertString.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/ImGuiDebugManager.h>
#include <Utility/StringUtils/StringUitls.h>
#include <cassert>


TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize()
{
    dxCommon_ = DXCommon::GetInstance();
    assert(dxCommon_);

	srvManager_ = SRVManager::GetInstance();

    Load("white.png");
    Load("cube.jpg");
    Load("uvChecker.png");

	needSort_ = true;
}

uint32_t TextureManager::Load(const std::string& _filepath, const std::string& defaultDirpath_)
{
	assert(dxCommon_ != nullptr && "not initialized");

	std::string fullpath = defaultDirpath_ + _filepath;

	return LoadTexture(fullpath);

}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(uint32_t _textureHandle)
{
	// テクスチャハンドルががが
	if (textures_.size() <= _textureHandle)
		return srvManager_->GetGPUSRVDescriptorHandle(_textureHandle);

	else
		return srvManager_->GetGPUSRVDescriptorHandle(textures_[_textureHandle].srvIndex);
}

Vector2 TextureManager::GetTextureSize(uint32_t _textureHandle)
{
	auto desc = textures_[_textureHandle].resource->GetDesc();
	Vector2 size;
	size.x = static_cast<float>(desc.Width);
	size.y = static_cast<float>(desc.Height);
	return size;
}

uint32_t TextureManager::LoadTexture(const std::string& _filepath)
{
	auto result = IsTextureLoaded(_filepath);
	if (result.has_value())
		return result.value();

	uint32_t srvIndex = srvManager_->Allocate();
	uint32_t index = static_cast<uint32_t>(textures_.size());
	DirectX::ScratchImage mipImages = GetMipImage(_filepath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textures_[index].resource = CreateTextureResource(metadata);
	textures_[index].intermediateResource = UploadTextureData(textures_[index].resource.Get(), mipImages);

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures_[index].srvIndex = srvIndex;

    std::string ext = _filepath.substr(_filepath.find_last_of(".") + 1);
	if (ext == "dds")
    {
        srvManager_->CreateSRVForCubemap(srvIndex, textures_[index].resource.Get(), metadata.format);
    }
    else
    {
        srvManager_->CreateSRVForTexture2D(srvIndex, textures_[index].resource.Get(), metadata.format, UINT(metadata.mipLevels));
    }

	//dxCommon_->GetDevice()->CreateShaderResourceView(, &srvDesc, srvManager_->GetCPUSRVDescriptorHandle(index));

	//キーの保存
	keys_[_filepath] = index;

    needSort_ = true;
	return static_cast<uint32_t>(index);
}

std::optional<uint32_t>  TextureManager::IsTextureLoaded(const std::string& _filepath)
{
	auto it = keys_.find(_filepath);
	if (it != keys_.end())
		return (*it).second;

	return std::nullopt;
}

DirectX::ScratchImage TextureManager::GetMipImage(const std::string& _filepath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathw = ConvertString(_filepath);
    HRESULT hr = S_FALSE;
	if (filePathw.ends_with(L".dds"))
	{
        hr = DirectX::LoadFromDDSFile(filePathw.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, nullptr, image);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

    if (FAILED(hr))
    {
		std::string str = "Failed to load texture: " + _filepath + "\n";
        Debug::Log(str);
		assert("Failed to load texture" && false);
		throw std::runtime_error("Failed to load texture");
		return {};
    }


    auto metadata = image.GetMetadata();
	if (metadata.mipLevels <= 1)
    {
		return image;
    }

	//ミップマップの生成
	DirectX::ScratchImage mipImage{};
	if (DirectX::IsCompressed(image.GetMetadata().format))
	{
		mipImage = std::move(image);
	}
	else
	{
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImage);
	}

    if (FAILED(hr))
    {
        std::string str = "Failed to generate mipmaps for texture: " + _filepath;
        Debug::Log(str);
		throw std::runtime_error("Failed to generate mipmaps for texture");
		return {};
    }


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
	ID3D12Device* device = dxCommon_->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetLoadCommandList();
	ID3D12CommandAllocator* allocator = dxCommon_->GetLoadCommandAllocator();
	ID3D12CommandQueue* queue = dxCommon_->GetCommandQueue();


	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, _mipImages.GetImages(), _mipImages.GetImageCount(), _mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(_texture, 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->CreateBufferResource(static_cast<uint32_t>(intermediateSize));


	UpdateSubresources(commandList, _texture, intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	//Tetureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = _texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	HRESULT hr = S_FALSE;
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* commandLists[] = { commandList };
	queue->ExecuteCommandLists(1, commandLists);
	dxCommon_->WaitForGPU();

	//次のフレーム用のコマンドリストを準備
	hr = allocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(allocator, nullptr);
	assert(SUCCEEDED(hr));

	return intermediateResource;
}

TextureManager::TextureManager()
{
#ifdef _DEBUG
	ImGuiDebugManager::GetInstance()->RegisterMenuItem("TextureManager", [this](bool* _open) {ImGui(_open); });
#endif // _DEBUG
}

TextureManager::~TextureManager()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow("TextureManager");
#endif // _DEBUG
}

void TextureManager::ImGui(bool* _open)
{
#ifdef _DEBUG
    ImGui::Begin("TextureManager",_open);
    ImGui::PushID(this);

	std::list<std::pair<std::string, uint32_t>> keysList(keys_.begin(), keys_.end());
    if (needSort_)
	{
		keysList.sort([](const std::pair<std::string, uint32_t>& a, const std::pair<std::string, uint32_t>& b) {
			return a.second < b.second; // valueでソート
			});
        needSort_ = false; // ソート後はフラグをリセット
	}

	uint32_t hoveredIndex = UINT32_MAX;
    for (const auto& key : keysList)
    {
        ImGui::Text("%d: %s", key.second, key.first.c_str());
		if (ImGui::IsItemHovered())
		{
			std::string extension = StringUtils::GetExtension(key.first);
			ImGui::SameLine();
            if (extension == "dds")
            {
                ImGui::Text("Type: CubeMap");
            }
            else
            {
                ImGui::Text("Type: Texture2D");
				hoveredIndex = key.second;
            }
		}
    }
    if (hoveredIndex != UINT32_MAX)
    {
		ImGui::Image(GetGPUHandle(hoveredIndex).ptr, ImVec2(100, 100));
    }

	ImGui::PopID();
    ImGui::End();
#endif // _DEBUG
}
