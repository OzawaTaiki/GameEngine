#include "DXCommon.h"

#include "WinApp.h"
#include "Debug.h"
#include "PSOManager.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include <cassert>
#include <thread>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

//#include "externals/imgui/imgui.h"
//#include "externals/imgui/imgui_impl_dx12.h"
//#include "externals/imgui/imgui_impl_win32.h"


DXCommon* DXCommon::GetInstance()
{
	static D3DResourceLeakChecker leakcheker;
    static DXCommon instance;
    return &instance;
}

void DXCommon::Initialize(WinApp* _winApp, int32_t _backBufferWidth, int32_t _backBufferHeight)
{
	assert(_winApp);

	winApp_ = _winApp;
	backBufferWidth_ = _backBufferWidth;
	backBufferHeight_ = _backBufferHeight;

	InitializeFixFPS();
	CreateDevice();
	InitializeCommand();
	CreateSwapChain();
	CreateDepthBuffer();
	CreateDescriptor();
	InitializeRenderTarget();
	InitializeDepthStencilView();
	CreateFence();
	InitializeViewport();
	CreateScissorRect();
	CreateDXcCompiler();
	InitializeImGui();
}

void DXCommon::PreDraw()
{
	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//trasitionBarrierを貼るコード
	//今回のバリアはtransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex].Get();
	//遷移前（現在）のResourceState
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//transitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);


	//描画先のRTVを設定する
	//指定した色で画面算体をクリアする


	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//描画先とRTVとDSVの設定を行う
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvDescriptorHeap_.Get(), desriptorSizeDSV, 0);
	commandList_->OMSetRenderTargets(1, &RTVHandles_[backBufferIndex], false, &dsvHandle);

	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(RTVHandles_[backBufferIndex], clearColor, 0, nullptr);

	//指定した深度で画面をクリアする
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList_->RSSetViewports(1, &viewport_);                      // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_);                      // Scissorを設定

	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DXCommon::PostDraw()
{
	HRESULT hr = S_FALSE;

	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//画面に書く処理はすべて終わり，画面に移すので状態を遷移
	//今回はRenderTargetからPresentにする
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);

	//コマンドリストの内容を確立させる。すべてのコマンドを積んでからcloseすること
	hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	/// コマンドをキックする
	//GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists->GetAddressOf());
	//GPUとOSに画面の交換を行うように通知する
	swapChain_->Present(1, 0);			//	画面が切り替わる

	/// GPUにSignalを送る
	//Fenceの値の更新
	fenceValue_++;
	//GPUがここまでたどり着いたときに，Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	//Fenceの値が指定したSignal値にたどり着いているか確認する
	//GetCompleteValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		//指定したSignalにたどり着いていないので，たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		//イベント待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	UpdateFixFPS();

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCommon::CreateBufferResource(uint32_t _sizeInBytes)
{
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;               // UploadHeapを使う


	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合は別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = _sizeInBytes;
	//バッファの場合はこれらを１にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource;
	HRESULT hr = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
												  &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												  IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));

	return bufferResource;
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
			Utils::Log(Utils::ConvertString(std::format(L"\nUse Adapater:{}\n", adapterDesc.Description)));
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
			Utils::Log(std::format("\nFeatureLevel : {}\n", featureLvelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	//初期化完了のログ
	Utils::Log("Complete create D3D12Device\n");

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
	swapChainDesc_.Width = backBufferWidth_;                         //画面の幅。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc_.Height = backBufferHeight_;                       //画面の高さ。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;              //色の形式
	swapChainDesc_.SampleDesc.Count = 1;                             //マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    //描画ターゲットとして利用すbる
	swapChainDesc_.BufferCount = 2;                                  //ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       //モニタにうつしたら中身を破棄
	//コマンドキュー，ウィンドウハンドル，設定を渡して生成する
	hresult = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hresult));
}

void DXCommon::CreateDepthBuffer()
{
	depthStencilResource_ = CreateDepthStencilTextureResource(WinApp::kWindowWidth_, WinApp::kWindowHeight_);
}

void DXCommon::CreateDescriptor()
{
	//DescriptorSizeを取得しておく
	desriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	desriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないのでShaderVisibleはfalse
	rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);              //viewの情報を格納している場所(Discriptor)の束(配列)

	////imguiを使うためSRV用のが必要
	////SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなのでShaderVisivleはtrue
	//srvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

}

void DXCommon::InitializeRenderTarget()
{
	HRESULT hresult = S_FALSE;

	uint32_t RTVSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	/// RTVの生成
	// 設定
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;         //出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;    //2dテクスチャとして書き込む

	// 生成
	for (uint32_t i = 0; i < 2; i++)
	{
		hresult = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hresult));

		RTVHandles_[i] = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
		RTVHandles_[i].ptr += RTVSize * i;

		device_->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc_, RTVHandles_[i]);
	}

}

void DXCommon::InitializeDepthStencilView()
{
	 dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//基本的にはresourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
	//DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
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

void DXCommon::InitializeViewport()
{
	// ビューポート領域のサイズを一緒にして画面全体を表示
	viewport_.Width = WinApp::kWindowWidth_;
	viewport_.Height = WinApp::kWindowHeight_;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

}

void DXCommon::CreateScissorRect()
{
	// シザー矩形
	// scissorRect.left – ビューポートと同じ幅と高さに設定されることが多い
	scissorRect_.left = 0;
	scissorRect_.right = WinApp::kWindowWidth_;
	scissorRect_.top = 0;
	scissorRect_.bottom = WinApp::kWindowHeight_;
}

void DXCommon::CreateDXcCompiler()
{
//	HRESULT hr = S_FALSE;
//
//	/// DXCの初期化
//	// dxcCompilerを初期化
//	dxcUtils_ = nullptr;
//	dxcCompiler_ = nullptr;
//	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
//	assert(SUCCEEDED(hr));
//	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
//	assert(SUCCEEDED(hr));
//
//	//現時点でinclideしないが,includeに対応するための設定を行っておく
//	 includeHandler_ = nullptr;
//	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
//	assert(SUCCEEDED(hr));
}

void DXCommon::InitializeImGui()
{/*
	///imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(
		device_.Get(),
		swapChainDesc_.BufferCount,
		rtvDesc_.Format,
		srvDescriptorHeap_.Get(),
		srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart()
	);*/
}

void DXCommon::InitializeFixFPS()
{
	reference_ = std::chrono::steady_clock::now();
}

void DXCommon::UpdateFixFPS()
{
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / 60.0f));
	const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / 65.0f));

	// 現在時間を取得
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	// 前回記録からの経過時間を取得
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒(よりわずかに短い時間)経っていないとき
	if (elapsed < kMinTime) {
		//1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			//1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	//現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;              //viewの情報を格納している場所(Discriptor)の束(配列)
	D3D12_DESCRIPTOR_HEAP_DESC descRiptorHeapDesc{};
	descRiptorHeapDesc.Type = _heapType;    //レンダーターゲットビュー(RTV)用
	descRiptorHeapDesc.NumDescriptors = _numDescriptors;                       //ダブルバッファ用に２つ。多くもかまわない
	descRiptorHeapDesc.Flags = _shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device_->CreateDescriptorHeap(&descRiptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプターヒープが生成できなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCommon::CreateDepthStencilTextureResource( int32_t _width, int32_t _height)
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = _width; // Textureの幅
	resourceDesc.Height = _height; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、通常は1
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う指定
	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	//震度のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット，Resourceと合わせる

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特別な設定は特になし。
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
		&depthClearValue, // Clear値の値
		IID_PPV_ARGS(resource.GetAddressOf())); // 作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));


	return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (_descriptorSize * _index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (_descriptorSize * _index);
	return handleGPU;
}