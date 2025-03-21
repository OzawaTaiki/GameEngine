#pragma once

#include <Core/DXCommon/LeakChecker/D3DResourceLeakChecker.h>
#include <Math/Vector/Vector4.h>
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

    void WaitForGPU();

    void ExecuteCommandList();

	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
	ID3D12GraphicsCommandList* GetLoadCommandList() { return loadCommanList_.Get(); }

	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_.Get(); }
	ID3D12CommandAllocator* GetLoadCommandAllocator() { return loadCommandAllocator_.Get(); }
	ID3D12Device* GetDevice() { return device_.Get(); }

	size_t GetBackBufferSize() const { return swapChainDesc_.BufferCount; }
    UINT GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

    IDXGISwapChain4* GetSwapChain() { return swapChain_.Get(); }
    ID3D12Resource* GetSwapChainResource(size_t _index) { return swapChainResources_[_index].Get(); }

    void ChangeState(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _before, D3D12_RESOURCE_STATES _after);

    void SetClearColor(float _r, float _g, float _b, float _a) { clearColor_[0] = _r; clearColor_[1] = _g; clearColor_[2] = _b; clearColor_[3] = _a; }
    void SetClearColor(float _color[4]) { clearColor_[0] = _color[0]; clearColor_[1] = _color[1]; clearColor_[2] = _color[2]; clearColor_[3] = _color[3]; }

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(uint32_t _sizeInBytes);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateUAVBufferResource(uint32_t _sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateReadbackResources(size_t _sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible);

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVDescriptorHandle(uint32_t _index) { return GetCPUDescriptorHandle(dsvDescriptorHeap_.Get(), desriptorSizeDSV_, _index); }

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
    void CreateRenderTexture();


	void InitializeFixFPS();
	void UpdateFixFPS();


	WinApp* winApp_ = nullptr;

	int32_t backBufferWidth_;
	int32_t backBufferHeight_;


	uint32_t desriptorSizeSRV_;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> loadCommandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> loadCommanList_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
    float rtClearValue_[4] = { 1.0f,0.0f,0.0f,1.0f };


	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};


	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
	HANDLE fenceEvent_;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	uint32_t desriptorSizeDSV_;
	D3D12_RESOURCE_BARRIER barrier_{};

	std::chrono::steady_clock::time_point reference_ = {};

    float clearColor_[4] = { 0.4625f,0.925f,0.4625f,1.0f };

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t _width, int32_t _height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);

	DXCommon() = default;
	~DXCommon() = default;
	DXCommon(const DXCommon&) = delete;
	const DXCommon& operator=(const DXCommon&) = delete;

};
