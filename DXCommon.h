#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include <cstdint>
#include <wrl.h>

class WinApp;

class DXCommon
{
public:
	static DXCommon* GetInstance();

	void Initialize(WinApp* _winApp, int32_t _backBufferWidth, int32_t _backBufferHeight);

private:

	void CreateDevice();

	void InitializeCommand();

	void CreateSwapChain();

	void CreateRenderTarget();

	void CreateFence();

	WinApp* winApp_ = nullptr;

	int32_t backBufferWidth_;
	int32_t backBufferHeight_;


	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles_[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
	HANDLE fenceEvent_;

};