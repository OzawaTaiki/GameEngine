#pragma once

#include "D3DResourceLeakChecker.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include <chrono>
#include <cstdint>

#include <wrl.h>
class WinApp;

class DXCommon
{
public:
	static DXCommon* GetInstance();

	void Initialize(WinApp* _winApp, int32_t _backBufferWidth, int32_t _backBufferHeight);

	void PreDraw();
	void PostDraw();

	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
	ID3D12Device* GetDevice() { return device_.Get(); }

	size_t GetBackBufferSize() const { return swapChainDesc_.BufferCount; }

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(uint32_t _sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible);
private:

	void CreateDevice();
	void InitializeCommand();
	void CreateSwapChain();
	void CreateDepthBuffer();
	void CreateDescriptor();
	void InitializeRenderTarget();
	void InitializeDepthStencilView();
	void CreateFence();
	void InitializeViewport();
	void CreateScissorRect();
	void CreateDXcCompiler();
	void InitializeImGui();

	void InitializeFixFPS();
	void UpdateFixFPS();


	WinApp* winApp_ = nullptr;

	int32_t backBufferWidth_;
	int32_t backBufferHeight_;


	uint32_t desriptorSizeSRV_;
	uint32_t desriptorSizeRTV_;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles_[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
	HANDLE fenceEvent_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	uint32_t desriptorSizeDSV_;
	D3D12_RESOURCE_BARRIER barrier_{};

	std::chrono::steady_clock::time_point reference_ = {};

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t _width, int32_t _height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);

	DXCommon() = default;
	~DXCommon() = default;
	DXCommon(const DXCommon&) = delete;
	const DXCommon& operator=(const DXCommon&) = delete;

};