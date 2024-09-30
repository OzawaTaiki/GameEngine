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

	ID3D12Device* GetDevice() { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }

private:

	void CreateDevice();

	void InitializeCommand();

	void CreateSwapChain();

	void CreateRenderTarget();

	void CreateFence();

	WinApp* winApp_ = nullptr;

	int32_t backBufferWidth_;
	int32_t backBufferHeight_;


	Microsoft::WRL::ComPtr<IDXGIFactory7>					dxgiFactory_				= nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4>					useAdapter_					= nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device>					device_						= nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>				commandQueue_				= nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>		commandList_				= nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>			commandAllocator_			= nullptr;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue>					infoQueue_					= nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			rtvDescriptorHeap_			= nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			srvDescriptorHeap_			= nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>					swapChain_					= nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>					swapChainResources_[2]		= {};
	Microsoft::WRL::ComPtr<ID3D12Fence>						fence_						= nullptr;


	D3D12_CPU_DESCRIPTOR_HANDLE								RTVHandles_[2]				={};
	uint64_t												fenceValue_					= 0;
	HANDLE													fenceEvent_					= nullptr;

};