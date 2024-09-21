#include "DXCommon.h"

#include "WinApp.h"
#include "Debug.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")


DXCommon* DXCommon::GetInstance()
{
    static DXCommon instance;
    return &instance;
}

void DXCommon::Initialize(WinApp* _winApp, int32_t _backBufferWidth, int32_t _backBufferHeight)
{
	winApp_ = _winApp;
	backBufferWidth_ = _backBufferWidth;
	backBufferHeight_ = _backBufferHeight;


	// デバイスの生成
	CreateDevice();

	// コマンドアロケータの生成
	// コマンドリストの生成
	// コマンドキューの生成
	InitializeCommand();

	// スワップチェーンの生成
	CreateSwapChain();

	// レンダーターゲットビューの生成
	CreateRenderTarget();

	// フェンスの生成
	CreateFence();

	// コマンドリストを閉じる
	commandList_->Close();
}


void DXCommon::CreateDevice()
{
	HRESULT hresult = S_FALSE;

#ifdef _DEBUG
	// デバックレイヤー
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU制御側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif // _DEBUG

	hresult = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hresult));

	/// 使用するアダプタ(GPU)を決定する
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hresult = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hresult));//取得でき何のは一大事
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//採用したアダプタの情報をログに出力。wstringの方なので注意。
			Log(ConvertString(std::format(L"\nUse Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		//ソフトウェアアダプタの場合は見なかったことにする
		useAdapter_ = nullptr;
	}
	//適切なアダプターが見つからなかったので起動できない
	assert(useAdapter_ != nullptr);


	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLvelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); i++)
	{
		//採用したアダプタでデバイスを生成
		hresult = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		//指定したレベルでデバイスが生成で来たかを確認
		if (SUCCEEDED(hresult))
		{
			//生成で来たのでログの出力を行ってループを抜ける
			Log(std::format("\nFeatureLevel : {}\n", featureLvelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	//初期化完了のログ
	Log("Complete create D3D12Device\n");

#ifdef _DEBUG

	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue_))))
	{
		//やばいエラー時に止まる
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に泊まる
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			//URL省略
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue_->PushStorageFilter(&filter);
	}
#endif // _DEBUG

}

void DXCommon::InitializeCommand()
{
	HRESULT hresult = S_FALSE;

	/// コマンドアロケータを生成
	hresult = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hresult));

	/// コマンドリストを生成
	hresult = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hresult));

	/// コマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hresult = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hresult));
}

void DXCommon::CreateSwapChain()
{
	HRESULT hresult = S_FALSE;

	/// SwapChainを生成する
	//スワップチェーンを生成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = backBufferWidth_;                         //画面の幅。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc.Height = backBufferHeight_;                       //画面の高さ。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;              //色の形式
	swapChainDesc.SampleDesc.Count = 1;                             //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    //描画ターゲットとして利用すbる
	swapChainDesc.BufferCount = 2;                                  //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       //モニタにうつしたら中身を破棄
	//コマンドキュー，ウィンドウハンドル，設定を渡して生成する
	hresult = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hresult));
}

void DXCommon::CreateRenderTarget()
{
	HRESULT hresult = S_FALSE;

	/// ディスクリプタヒープを生成
	// 設定
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// 生成
	hresult = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	assert(SUCCEEDED(hresult));

	uint32_t RTVSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	/// RTVの生成
	// 設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;         //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;    //2dテクスチャとして書き込む

	// 生成
	for (uint32_t i = 0; i < 2; i++)
	{
		hresult = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hresult));

		RTVHandles_[i] = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
		RTVHandles_[i].ptr += RTVSize * i;

		device_->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc, RTVHandles_[i]);
	}

}

void DXCommon::CreateFence()
{
	HRESULT hresult = S_FALSE;

	fenceValue_ = 0;
	hresult = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hresult));

	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}
