#pragma once

#include <vector>
#include <string>

#include <d3d12.h>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"

class TextureManager
{
	struct Texture
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandlerCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandlerGPU;
		std::string name;
	};

public:
	static TextureManager* GetInstance();

	void Initilize(ID3D12Device* _device, const std::string& _directoryPath = "Resources/");

	static uint32_t Load(const std::string& _fileName,ID3D12GraphicsCommandList* _cl);

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(uint32_t _textureHandle);

	ID3D12DescriptorHeap* GetDescriptorHeap() { return descriptorHeap_.Get(); }

private:
	ID3D12Device* device_ = nullptr;
	std::string defaultDirectoryPath_ = {};

	std::vector<Texture> textures_ = {};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
	uint32_t srvDescroptorSize_ = 0;


	DirectX::ScratchImage LoadTexture(const std::string& _fileName);

	void CreateSrv(const DirectX::ScratchImage& _mipImages, ID3D12GraphicsCommandList* _cl);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateResource(const DirectX::TexMetadata& _metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* _cl);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t _sizeInBytes);
	uint32_t FindTexture(const std::string& _fileName);
};