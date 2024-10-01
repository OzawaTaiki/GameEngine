#include <Windows.h>
#include <string>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include "myLib/MyLib.h"
#include "Debug.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <fstream>
#include <sstream>

#include <random>
#include <numbers>

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);
//
//void Log(const std::string& message);
//std::wstring ConvertString(const std::string& _str);
//std::string ConvertString(const std::wstring& _str);

template <class T>
void pLog(const std::string& _objname, Microsoft::WRL::ComPtr<T> _p)
{
	char buffer[256];
	sprintf_s(buffer, "address: %p : %s \n", _p.Get(), _objname.c_str());
	OutputDebugStringA(buffer);
}
void pLog(const std::string& _objname, ID3D12Resource* _p)
{
	char buffer[256];
	sprintf_s(buffer, "address: %p : %s \n", &_p, _objname.c_str());
	OutputDebugStringA(buffer);
}


// クライアント領域のサイズ
const int32_t kClientWidth = 1280;
const int32_t kClientHeight = 720;

const float kDeltaTime = 1.0f / 60.0f;


Microsoft::WRL::ComPtr<IDxcBlob> ComplieShader(
	//Complierするshaderファイルへのパス
	const std::wstring& _filePath,
	//Compilerに使用するprofile
	const wchar_t* _profile,
	//初期化で生成したものを3つ
	Microsoft::WRL::ComPtr<IDxcUtils>& _dxcUtils,
	Microsoft::WRL::ComPtr<IDxcCompiler3>& _dxcCompiler,
	Microsoft::WRL::ComPtr<IDxcIncludeHandler>& _includeHandler);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, size_t _sizeInBytes);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, int32_t _width, int32_t _height);

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible);

//textureデータを読む
DirectX::ScratchImage LoadTexture(const std::string& _filePath);

//DirectX13のtextureリソースを作る
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const DirectX::TexMetadata& _metadata);

//データを転送するUploadTextureData関数を作る
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& _texture, const DirectX::ScratchImage& _mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList);


D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);



struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;		//法線
};

struct Material
{
	Vector4 color;
	int32_t enabledLighthig;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};

struct DirectionalLight
{
	Vector4 color;		//ライトの色
	Vector3 direction;	//ライトの向き
	float intensity;	//輝度
	uint32_t isHalf;
	float pad[3];
};

struct PointLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;	// ライトの影響半径
	float decay;	// 減衰率
	uint32_t isHalf;
	float pad;
};

struct SpotLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;	// 減衰率
	float cosAngle;
	float falloutStartAngle;
	uint32_t isHalf;
};

struct CameraForGPU
{
	Vector3 worldPosition;
};

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 worldInverseTranspose;
};

struct Particle
{
	stTransform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct Emitter
{
	stTransform transform;  //
	uint32_t count;			//発生数
	float frequency;		//発生頻度
	float frequencyTime;	//頻度用時刻
};


struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct AccelerationField
{
	Vector3 acceleration;
	AABB area;
};

struct Node
{
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> chirdren;
};

Node ReadNode(aiNode* _node);

struct ModelData
{
	std::vector<VertexData> vertices;
	std::string textureHandlePath;

	TransformationMatrix* transformMat;
	VertexData* vertexData;
	Material* materialData;
	Node rootNode;
	float* useTexture;
	uint32_t* indexData;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> useTextureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	uint32_t vertexNum;
	uint32_t indexNum;
	uint32_t textureHandle;
};

struct  Object
{
	TransformationMatrix* transformMat;
	VertexData* vertexData;
	Material* materialData;
	float* useTexture;
	uint32_t* indexData;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> useTextureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	uint32_t vertexNum;
	uint32_t indexNum;
	uint32_t textureHandle;
};

struct Texture
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandlerCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandlerGPU;
	std::string name;
};

std::vector<Texture> textures;
void DeleteTextures();

std::unique_ptr<ModelData> LoadObjFile(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize);

std::unique_ptr<ModelData> LoadObjFile(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device);

std::unique_ptr<ModelData> LoadObjFileWithAssimp(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize);

std::unique_ptr<ModelData> LoadObjFileWithAssimp(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device);


/// <summary>
/// 読み込んだテクスチャを取り出す
/// </summary>
/// <param name="_textureHandle">テクスチャハンドル</param>
/// <returns>テクスチャデータ</returns>
D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(uint32_t _textureHandle);

/// <summary>
/// テクスチャを読み込む
/// </summary>
/// <param name="_filePath">ファイルパス</param>
/// <param name="_device">デバイス</param>
/// <param name="_commandList">コマンドリスト</param>
/// <param name="_srvDescriptorHeap">ｓｒｖディスクリプタヒープ</param>
/// <param name="_srvSize">srvのサイズ</param>
/// <returns>テクスチャの登録番号</returns>
uint32_t LoadTexture(const std::string& _filePath, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize);

/// <summary>
/// 三角形のデータ作成
/// </summary>
/// <param name="_device">デバイス</param>
/// <param name="_obj">データ格納用Object変数</param>
std::unique_ptr<Object>  MakeTriangleData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device);

/// <summary>
/// 球のデータ作成
/// </summary>
/// <param name="_device">デバイス</param>
/// <param name="_obj">データ格納用Object変数</param>
std::unique_ptr<Object>  MakeSphereData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device);

/// <summary>
/// スプライトのデータ作成
/// </summary>
/// <param name="_device">デバイス</param>
/// <param name="_obj">データ格納用Object変数</param>
std::unique_ptr<Object>   MakeSpriteData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device);

std::unique_ptr<ModelData> MakeModelData(Microsoft::WRL::ComPtr<ID3D12Device>& _device,  const std::string& _directoryPath, const std::string& _filename);

void InitializeMeshData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, ModelData* _model);

/// <summary>
/// スプライトのTransformationMatrixの計算
/// </summary>
/// <param name="_transform">トランスフォームデータ</param>
/// <returns>WVPt および WorldMat</returns>
TransformationMatrix CalculateSpriteWVPMat(const stTransform& _transform);

TransformationMatrix CalculateObjectWVPMat(const stTransform& _transform, const Matrix4x4& _VPmat);

/// <summary>
/// 三角形の描画
/// </summary>
/// <param name="_commandList">コマンドリスト</param>
/// <param name="_obj">三角形のデータ作成したObject変数</param>
/// <param name="_textureHandle">テクスチャハンドル</param>
void DrawTriangle(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, Microsoft::WRL::ComPtr<ID3D12Resource> _light, uint32_t _textureHandle = 0);

/// <summary>
/// スプライトの描画
/// </summary>
/// <param name="_commandList">コマンドリスト</param>
/// <param name="_obj">スプライトのデータ作成したObject変数</param>
/// <param name="_textureHandle">テクスチャハンドル</param>
void DrawSprite(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, Microsoft::WRL::ComPtr<ID3D12Resource> _light, uint32_t _textureHandle = 0);

void DrawSphere(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, uint32_t _textureHandle = 0);

Particle MakeNewParticle(std::mt19937& _randomEngine, const Emitter& _emitter);

std::list<Particle> Emit(const Emitter& _emitter, std::mt19937& _randomEngine);

enum class BlendMode
{
	kBlendModeNormal,
	kBlendModeAdd,
	kBlendModeSub,
	kBlendModeMultiply,
	kBlendModeScreen

};

void SetBlendMode(BlendMode _blendMode, D3D12_GRAPHICS_PIPELINE_STATE_DESC& _graphicsPipelineStateDesc);

TransformationMatrix CalculateParticleWVPMat(const stTransform& _transform, const Matrix4x4& _cameraMatrix, const Matrix4x4& _VPmat, bool useBillborad);

struct D3DResourceLeakChecker
{
	~D3DResourceLeakChecker()
	{
		//リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	D3DResourceLeakChecker leakcheker;

	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	///COMの初期化	CoInitializeEx(0, COINIT_MULTITHREADED);

	/// ウィンドウクラスを登録する
	WNDCLASS wc{};
	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名(なんでもいい)
	wc.lpszClassName = L"CGWindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	/// ウィンドウサイズを決める

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域をもとに実際のサイズをwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	/// ウィンドウを生成して表示
	// ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		// 利用するクラス名
		L"CG2",					// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル
		CW_USEDEFAULT,			// 表示X座標(WindowsにOS任せる)
		CW_USEDEFAULT,			// 表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left,	// ウィンドウ横幅
		wrc.bottom - wrc.top,	// ウィンドウ立幅
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		wc.hInstance,			// インスタンスハンドル
		nullptr);				// オプション

	//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	///デバッグレイヤー
#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU制御側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif // _DEBUG


	/// DXGIFactoeyの生成
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// HRESULTはWindows系のエラーコードであり，
	// 関数が成功したかどうかをSUCCEEDE各炉で判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違がっているか，どうにもできない場合が多いので
	// assertしておく
	assert(SUCCEEDED(hr));


	/// 使用するアダプタ(GPU)を決定する
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得でき何のは一大事
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//採用したアダプタの情報をログに出力。wstringの方なので注意。
			Log(ConvertString(std::format(L"\nUse Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		//ソフトウェアアダプタの場合は見なかったことにする
		useAdapter = nullptr;
	}
	//適切なアダプターが見つからなかったので起動できない
	assert(useAdapter != nullptr);



	///D3D12Deviceの生成
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

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
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		//指定したレベルでデバイスが生成で来たかを確認
		if (SUCCEEDED(hr))
		{
			//生成で来たのでログの出力を行ってループを抜ける
			Log(std::format("\nFeatureLevel : {}\n", featureLvelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	//初期化完了のログ
	Log("Complete create D3D12Device\n");

	///エラー警告即停止
#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{

		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に泊まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//解放
		//infoQueue->Release();

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
		infoQueue->PushStorageFilter(&filter);

	}

#endif // _DEBUG



	/// CommandQueueを生成する
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;         //commandListをGPUに投げて実行する人
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	/// commandListを生成する
	//コマンドアロケータを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr; //命令保存用のメモリ管理機構 commandListとワンセット
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;           //まとまった命令群
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	/// SwapChainを生成する
	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;                             //画面の幅。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;                           //画面の高さ。ウィンドウクラスのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;              //色の形式
	swapChainDesc.SampleDesc.Count = 1;                             //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    //描画ターゲットとして利用すbる
	swapChainDesc.BufferCount = 2;                                  //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       //モニタにうつしたら中身を破棄
	//コマンドキュー，ウィンドウハンドル，設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));


	///DescriptorHeapを生成する

	//DescriptorSizeを取得しておく
	const uint32_t desriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t desriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないのでShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);              //viewの情報を格納している場所(Discriptor)の束(配列)



	//imguiを使うためSRV用のが必要
	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなのでShaderVisivleはtrue
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);



	/// SwapChainからResourceを引っ張ってくる
	//SwapChainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));


	/// RTVを作る
	//rtvの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;         //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;    //2dテクスチャとして書き込む

	D3D12_CPU_DESCRIPTOR_HANDLE rtVHandles[2];
	rtVHandles[0] = GetCPUDescriptorHandle(rtvDescriptorHeap, desriptorSizeRTV, 0);
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtVHandles[0]);

	rtVHandles[1] = GetCPUDescriptorHandle(rtvDescriptorHeap, desriptorSizeRTV, 1);
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtVHandles[1]);


	///FenceとEventを生成
	//初期値０でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));


	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);


	/// DXCの初期化
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でinclideしないが,includeに対応するための設定を行っておく
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));




	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	//descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	//descriptionRootSignature.pStaticSamplers = staticSamplers;


	/// RootSignatrueを生成する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//descriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//０から始まる
	descriptorRange[0].NumDescriptors = 1;//数は１つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[8] = {};

	//マテリアル
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                           // レジスタ番号0を使う

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;                           // レジスタ番号0を使う

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

	//テクスチャの有無
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	//directinalLight
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 2;

	//カメラ
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[5].Descriptor.ShaderRegister = 3;

	//pointLight
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[6].Descriptor.ShaderRegister = 4;

	//spotlight
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[7].Descriptor.ShaderRegister = 5;


	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	/// InputLayoutの設定を行う
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	/// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	/// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	/// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ComplieShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = ComplieShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);



	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効にする
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqeul つまり近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/// PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();                                                 // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                               // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
	// 追加の DRTV の情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// どのように画面に色を打ち込むかの設定 (気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

#pragma region instancing

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureForInstancing{};
	descriptionRootSignatureForInstancing.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;//0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1;//数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParametersForInstancing[4] = {};
	rootParametersForInstancing[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
	rootParametersForInstancing[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
	rootParametersForInstancing[0].Descriptor.ShaderRegister = 0;                           // レジスタ番号0を使う

	rootParametersForInstancing[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;// CBVを使う
	rootParametersForInstancing[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       // VertexShaderで使う
	rootParametersForInstancing[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParametersForInstancing[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);


	rootParametersForInstancing[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
	rootParametersForInstancing[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
	rootParametersForInstancing[2].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
	rootParametersForInstancing[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

	rootParametersForInstancing[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
	rootParametersForInstancing[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
	rootParametersForInstancing[3].Descriptor.ShaderRegister = 1;                           // レジスタ番号1を使う


	descriptionRootSignatureForInstancing.pParameters = rootParametersForInstancing;
	descriptionRootSignatureForInstancing.NumParameters = _countof(rootParametersForInstancing);         // 配列の長さ

	descriptionRootSignatureForInstancing.pStaticSamplers = staticSamplers;
	descriptionRootSignatureForInstancing.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlobForInstancing = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlobForInstancing = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignatureForInstancing, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlobForInstancing, &errorBlobForInstancing);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(signatureBlobForInstancing->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignatureForInstancing = nullptr;
	hr = device->CreateRootSignature(0, signatureBlobForInstancing->GetBufferPointer(), signatureBlobForInstancing->GetBufferSize(), IID_PPV_ARGS(&rootSignatureForInstancing));
	assert(SUCCEEDED(hr));


	/// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> particleVSBlob = ComplieShader(L"Particle.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> particlePSBlob = ComplieShader(L"Particle.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDescForInstancing{};
	//Depthの機能を有効にする
	depthStencilDescForInstancing.DepthEnable = true;
	//書き込みします
	depthStencilDescForInstancing.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqeul つまり近ければ描画される
	depthStencilDescForInstancing.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/// PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDescForInstancing{};
	graphicsPipelineStateDescForInstancing.pRootSignature = rootSignatureForInstancing.Get();                                                 // RootSignature
	graphicsPipelineStateDescForInstancing.InputLayout = inputLayoutDesc;                                                        // InputLayout
	graphicsPipelineStateDescForInstancing.VS = { particleVSBlob->GetBufferPointer(), particleVSBlob->GetBufferSize() };	        // VertexShader
	graphicsPipelineStateDescForInstancing.PS = { particlePSBlob->GetBufferPointer(), particlePSBlob->GetBufferSize() };         // PixelShader
	//graphicsPipelineStateDescForInstancing.BlendState = blendDesc;                                                               // BlendState
	SetBlendMode(BlendMode::kBlendModeNormal, graphicsPipelineStateDescForInstancing);
	graphicsPipelineStateDescForInstancing.RasterizerState = rasterizerDesc;                                                     // RasterizerState
	// 追加の DRTV の情報
	graphicsPipelineStateDescForInstancing.NumRenderTargets = 1;
	graphicsPipelineStateDescForInstancing.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDescForInstancing.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDescForInstancing.DepthStencilState = depthStencilDescForInstancing;
	graphicsPipelineStateDescForInstancing.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// どのように画面に色を打ち込むかの設定 (気にしなくて良い)
	graphicsPipelineStateDescForInstancing.SampleDesc.Count = 1;
	graphicsPipelineStateDescForInstancing.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateForInstancing = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDescForInstancing, IID_PPV_ARGS(&graphicsPipelineStateForInstancing));
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region 平行光源
	DirectionalLight* directionalLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightData->isHalf = true;

#pragma endregion

#pragma region 点光源
	PointLight* pointLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>pointLightResource = CreateBufferResource(device, sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));

	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,2.0f,0.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 5.0f;
	pointLightData->decay = 0.5f;
	pointLightData->isHalf = true;
#pragma endregion


#pragma region spotlight
	SpotLight* spotLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>spotLightResource = CreateBufferResource(device, sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));

	spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData->position = { 3.0f,1.250f,0.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = Normalize({ -1.0f,-1.0f,0.0f });
	spotLightData->intensity = 4.0f;
	spotLightData->cosAngle = std::cos (std::numbers::pi_v<float> / 3.0f);
	spotLightData->falloutStartAngle = std::cos (std::numbers::pi_v<float> / 3.0f);
	spotLightData->decay = 2.0f;
	spotLightData->isHalf = true;
#pragma endregion

#pragma region OBjの読み込み

	////モデル読み込み
	//ModelData objModelData = LoadObjFile("resources/obj", "fence.obj", device, commandList, srvDescriptorHeap, desriptorSizeSRV);
	////頂点リソースを作る
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourcePlane = CreateBufferResource(device, sizeof(VertexData) * objModelData.vertices.size());
	////頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewPlane{};
	//vertexBufferViewPlane.BufferLocation = vertexResourcePlane->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	//vertexBufferViewPlane.SizeInBytes = UINT(sizeof(VertexData) * objModelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	//vertexBufferViewPlane.StrideInBytes = sizeof(VertexData);//1頂点あたりのサイズ

	////頂点リソースにデータを書き込む
	//VertexData* vertexData = nullptr;
	//vertexResourcePlane->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); //書き込むためのアドレスを取得
	//std::memcpy(vertexData, objModelData.vertices.data(), sizeof(VertexData) * objModelData.vertices.size());//頂点データをリソースにコピー

	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResorcePlane = CreateBufferResource(device, sizeof(Material));
	//Material* materialDataPlane;
	//materialResorcePlane->Map(0, nullptr, reinterpret_cast<void**>(&materialDataPlane));
	//materialDataPlane->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDataPlane->enabledLighthig = true;
	//materialDataPlane->uvTransform = MakeIdentity4x4();

	//// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> WvpMatrixResourcePlane = CreateBufferResource(device, sizeof(TransformationMatrix));
	//// データを書き込む
	//TransformationMatrix* WvpMatrixDataPlane = nullptr;
	//// 書き込むためのアドレスを取得
	//WvpMatrixResourcePlane->Map(0, nullptr, reinterpret_cast<void**>(&WvpMatrixDataPlane));
	//// 単位行列を書きこんでおく
	//WvpMatrixDataPlane->World = MakeIdentity4x4();



	//Microsoft::WRL::ComPtr<ID3D12Resource> texVisiblityPlane = CreateBufferResource(device, sizeof(float));
	//float* visiblePlane = nullptr;
	//texVisiblityPlane->Map(0, nullptr, reinterpret_cast<void**>(&visiblePlane));
	//*visiblePlane = 1.0f;
	//("texVisiblityPlane", texVisiblityPlane);

#pragma endregion


	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//基本的にはresourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());




	D3D12_VIEWPORT viewport{};
	// ビューポート領域のサイズを一緒にして画面全体を表示
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	// シザー矩形
	// scissorRect.left – ビューポートと同じ幅と高さに設定されることが多い
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;




	///imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(
		device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);


	///******************************************



	std::unique_ptr<ModelData> modelData;
	modelData = MakeModelData(device, "resources/obj", "plane.obj");

	const uint32_t kNumMaxInstance = 100;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = CreateBufferResource(device, sizeof(ParticleForGPU) * kNumMaxInstance);

	ParticleForGPU* instancingData = nullptr;
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	for (uint32_t index = 0; index < kNumMaxInstance; index++)
	{
		instancingData->World = MakeIdentity4x4();
		instancingData->WVP = MakeIdentity4x4();
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandlerCPU = GetCPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandlerGPU = GetGPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, 2);
	device->CreateShaderResourceView(instancingResource.Get(), &instancingSrvDesc, instancingSrvHandlerCPU);


	///******************************************

	///カメラ
	CameraForGPU* cameraForGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource = CreateBufferResource(device, sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU));

	///
	/// 変数宣言
	///

	stTransform cameraTransform{ {1.0f,1.0f,1.0f},{0.260f,0.0f,0.0f} ,{0.0f,3.0f,-10.0f} };

	Vector4 objColor1 = { 1.0f, 1.0f, 1.0f, 1.0f };

	stTransform transformObj{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	stTransform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	stTransform PLTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,2.0f,0.0f} };
	stTransform spriteTrans{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	stTransform spriteUVTrans{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	std::list <Particle> particles;
	bool useBillboard = false;

	Emitter emitter{};
	emitter.count = 3;
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;
	emitter.transform = {
		.scale = {1.0f,1.0f,1.0f},
		.rotate = {0.0f,0.0f,0.0f},
		.translate = {0.0f,0.0f,0.0f}
	};

	AccelerationField accelerationField;
	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f ,-1.0f ,-1.0f };
	accelerationField.area.max = { 1.0f , 1.0f , 1.0f };
	bool enableAccelerationField = false;


	uint32_t currentTexture = 0;
	const char* textureOption[] = { "uvChecker","cube","monsterBall" };

	int currentBlendMode = static_cast<int> (BlendMode::kBlendModeNormal);
	const char* blendModeOption[] = { "normal","add","sub","multi","screen" };

	bool enableLightting[3] = { true,true ,true };
	bool useTexture[3] = { true ,true,true };


	uint32_t uvGH = LoadTexture("resources/images/uvChecker.png", device, commandList, srvDescriptorHeap, desriptorSizeSRV);
	uint32_t cubeGH = LoadTexture("resources/images/cube.jpg", device, commandList, srvDescriptorHeap, desriptorSizeSRV);
	uint32_t ballGH = LoadTexture("resources/images/monsterBall.png", device, commandList, srvDescriptorHeap, desriptorSizeSRV);
	uint32_t fenceGH = LoadTexture("resources/obj/fence.png", device, commandList, srvDescriptorHeap, desriptorSizeSRV);

	modelData->textureHandle = LoadTexture("./resources/images/circle.png", device, commandList, srvDescriptorHeap, desriptorSizeSRV);


	std::unique_ptr <Object> sphere;
	sphere= MakeSphereData(device);


	std::unique_ptr <Object> pointLight;
	pointLight = MakeSphereData(device);

	std::unique_ptr <Object> sprite;
	sprite = MakeSpriteData(device);

	std::unique_ptr<ModelData> plane;
	plane = std::unique_ptr<ModelData>(LoadObjFileWithAssimp("resources/obj/plane_for_glTF", "plane.gltf", device));
	plane->textureHandle = LoadTexture(plane->textureHandlePath, device, commandList, srvDescriptorHeap, desriptorSizeSRV);

	std::unique_ptr <ModelData>  terrianModel;
	terrianModel = std::unique_ptr<ModelData>(LoadObjFileWithAssimp("resources/obj", "terrain.obj", device, commandList, srvDescriptorHeap, desriptorSizeSRV));
	terrianModel->textureHandle = LoadTexture(terrianModel->textureHandlePath, device, commandList, srvDescriptorHeap, desriptorSizeSRV);

	stTransform terrainTrans{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };


	///
	/// メインループ
	/// 
	MSG msg{};
	// ウィンドウのｘボタンが押されるまでループ
	while (msg.message != WM_QUIT)
	{
		// Windowにメッセージが来ていたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();


			///
			/// 更新処理ここから
			/// 


			//ImGui::ShowDemoWindow();

			ImGui::Begin("Window");
			//if (ImGui::CollapsingHeader("object"))
			//{
			if (ImGui::TreeNode("Camera"))
			{
				ImGui::DragFloat3("scale", &cameraTransform.scale.x, 0.01f);
				ImGui::DragFloat3("rotate", &cameraTransform.rotate.x, 0.01f);
				ImGui::DragFloat3("translate", &cameraTransform.translate.x, 0.01f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Sphere"))
			{
				ImGui::ColorEdit4("color", &objColor1.x);
				ImGui::SliderFloat("shininess", &sphere->materialData->shininess, 1.0f, 50.0f);
				ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
				ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
				ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
				ImGui::Checkbox("Lighting", &enableLightting[0]);
				ImGui::Checkbox("useTexture", &useTexture[0]);
				int currentTex = static_cast<int>(sphere->textureHandle);
				if (ImGui::Combo("texture", &currentTex, textureOption, IM_ARRAYSIZE(textureOption)))
				{
					sphere->textureHandle = static_cast<uint32_t> (currentTex);
				}
				sphere->materialData->color = objColor1;
				sphere->materialData->enabledLighthig = enableLightting[0];
				*sphere->useTexture = useTexture[0] ? 1.0f : 0.0f;
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("OBJ"))
			{
				ImGui::ColorEdit4("color", &objColor1.x);
				ImGui::DragFloat3("scale", &transformObj.scale.x, 0.01f);
				ImGui::DragFloat3("rotate", &transformObj.rotate.x, 0.01f);
				ImGui::DragFloat3("translate", &transformObj.translate.x, 0.01f);
				ImGui::Checkbox("Lighting", &enableLightting[1]);
				ImGui::Checkbox("useTexture", &useTexture[1]);
				int currentTex = static_cast<int>(currentTexture);
				if (ImGui::Combo("texture", &currentTex, textureOption, IM_ARRAYSIZE(textureOption)))
				{
					currentTexture = static_cast<uint32_t> (currentTex);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Sprite"))
			{
				ImGui::ColorEdit4("color", &objColor1.x);
				ImGui::DragFloat3("scale", &spriteTrans.scale.x, 0.01f);
				ImGui::DragFloat3("rotate", &spriteTrans.rotate.x, 0.01f);
				ImGui::DragFloat3("translate", &spriteTrans.translate.x, 1.0f);
				int currentTex = static_cast<int>(sprite->textureHandle);
				if (ImGui::Combo("texture", &currentTex, textureOption, IM_ARRAYSIZE(textureOption)))
				{
					sprite->textureHandle = static_cast<uint32_t> (currentTex);
				}
				sprite->materialData->color = objColor1;

				if (ImGui::TreeNode("uvTransform"))
				{
					ImGui::DragFloat2("uvTranslate", &spriteUVTrans.translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("uvScale", &spriteUVTrans.scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("uvRotate", &spriteUVTrans.rotate.z);
					sprite->materialData->uvTransform = MakeAffineMatrix(spriteUVTrans.scale, spriteUVTrans.rotate, spriteUVTrans.translate);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Terrain"))
			{
				ImGui::ColorEdit4("color", &terrianModel->materialData->color.x);
				ImGui::SliderFloat("shininess", &terrianModel->materialData->shininess, 1.0f, 50.0f);
				ImGui::DragFloat3("scale", &terrainTrans.scale.x, 0.01f);
				ImGui::DragFloat3("rotate", &terrainTrans.rotate.x, 0.01f);
				ImGui::DragFloat3("translate", &terrainTrans.translate.x, 0.01f);
				ImGui::Checkbox("Lighting", &enableLightting[2]);
				ImGui::Checkbox("useTexture", &useTexture[2]);
				terrianModel->materialData->enabledLighthig = enableLightting[2];
				*terrianModel->useTexture = useTexture[1] ? 1.0f : 0.0f;
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("DirectionalLight"))
			{
				ImGui::ColorEdit3("color", &directionalLightData->color.x);
				ImGui::DragFloat3("direction", &directionalLightData->direction.x, 0.01f);
				ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f);
				bool isHalf = directionalLightData->isHalf != 0;
				ImGui::Checkbox("isHalf", &isHalf);
				directionalLightData->isHalf = static_cast<uint32_t>(isHalf);

				directionalLightData->direction = Normalize(directionalLightData->direction);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("PointLight"))
			{
				ImGui::ColorEdit3("color", &pointLightData->color.x);
				if (ImGui::DragFloat3("position", &pointLightData->position.x, 0.01f))
				{
					PLTransform.translate = pointLightData->position;
				}
				ImGui::DragFloat("intensity", &pointLightData->intensity, 0.01f);
				ImGui::DragFloat("decay", &pointLightData->decay, 0.01f);
				ImGui::DragFloat("radius", &pointLightData->radius, 0.01f);
				bool isHalf = pointLightData->isHalf != 0;
				ImGui::Checkbox("isHalf", &isHalf);
				pointLightData->isHalf = static_cast<uint32_t>(isHalf);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("spotLight"))
			{
				ImGui::ColorEdit3("color", &spotLightData->color.x);
				if (ImGui::DragFloat3("position", &spotLightData->position.x, 0.01f))
				{
					//PLTransform.translate = spotLightData->position;
				}
				ImGui::DragFloat("distance", &spotLightData->distance, 0.01f);
				ImGui::DragFloat3("direction", &spotLightData->direction.x, 0.01f);
				ImGui::DragFloat("intensity", &spotLightData->intensity, 0.01f);
				ImGui::DragFloat("decay", &spotLightData->decay, 0.01f);
				ImGui::SliderAngle("cosAngle", &spotLightData->cosAngle, 0.01f, 180.0f);
				ImGui::SliderAngle("falloutStart", &spotLightData->falloutStartAngle, 0.01f,180.0f);
				bool isHalf = spotLightData->isHalf != 0;
				ImGui::Checkbox("isHalf", &isHalf);
				spotLightData->isHalf = static_cast<uint32_t>(isHalf);
				spotLightData->direction = Normalize(spotLightData->direction);

				ImGui::TreePop();
			}
			//}
			/*if (ImGui::Button("Add Particles"))
			{
				particles.splice(particles.end(), Emit(emitter, randomEngine));
			}
			ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
			ImGui::Checkbox("enableField", &enableAccelerationField);
			if (ImGui::Combo("BlendMode", &currentBlendMode, blendModeOption, IM_ARRAYSIZE(blendModeOption)))
			{
				SetBlendMode(static_cast<BlendMode>(currentBlendMode), graphicsPipelineStateDescForInstancing);
				device->CreateGraphicsPipelineState(&graphicsPipelineStateDescForInstancing, IID_PPV_ARGS(&graphicsPipelineStateForInstancing));
			}
			ImGui::Checkbox("useBillboard", &useBillboard);*/
			ImGui::End();

			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

			cameraForGPU->worldPosition = cameraTransform.translate;

			//pointLightData->position = Transform(pointLightPosition, viewProjectionMatrix);


			emitter.frequencyTime += kDeltaTime;
			if (emitter.frequency <= emitter.frequencyTime)
			{
				particles.splice(particles.end(), Emit(emitter, randomEngine));
				emitter.frequencyTime -= emitter.frequency;
			}

			uint32_t numInstance = 0;
			for (auto particleIterator = particles.begin(); particleIterator != particles.end();)
			{
				if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
					particleIterator = particles.erase(particleIterator);
					continue;
				}
				if (numInstance < kNumMaxInstance)
				{
					float  alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

					if (enableAccelerationField && IsCollision(accelerationField.area, (*particleIterator).transform.translate))
					{
						(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
					}

					(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
					(*particleIterator).currentTime += kDeltaTime;
					instancingData[numInstance].WVP = CalculateParticleWVPMat((*particleIterator).transform, cameraMatrix, viewProjectionMatrix, useBillboard).WVP;
					instancingData[numInstance].World = CalculateParticleWVPMat((*particleIterator).transform, cameraMatrix, viewProjectionMatrix, useBillboard).World;
					(*particleIterator).color.w = alpha;
					instancingData[numInstance].color = (*particleIterator).color;
					numInstance++;
				}

				particleIterator++;
			}
			
			//*WvpMatrixDataPlane = CalculateObjectWVPMat(transformObj, viewProjectionMatrix);


			transform.rotate.y += 0.001f;
			*sphere->transformMat = CalculateObjectWVPMat(transform, viewProjectionMatrix);
			*pointLight->transformMat = CalculateObjectWVPMat(PLTransform, viewProjectionMatrix);

			*sprite->transformMat = CalculateSpriteWVPMat(spriteTrans);
			*terrianModel->transformMat = CalculateObjectWVPMat(terrainTrans, viewProjectionMatrix);
			//*plane->transformMat = CalculateObjectWVPMat(terrainTrans, viewProjectionMatrix);
			Matrix4x4 wMat = MakeAffineMatrix(terrainTrans.scale, terrainTrans.rotate, terrainTrans.translate);
			plane->transformMat->WVP = plane->rootNode.localMatrix * wMat * viewProjectionMatrix;
			plane->transformMat->World = plane->rootNode.localMatrix * wMat ;
			plane->transformMat->worldInverseTranspose= Transpose(Inverse(plane->transformMat->World));

			///
			/// 更新処理ここまで
			///

			//これから書き込むバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			//trasitionBarrierを貼るコード
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはtransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを貼る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
			//遷移前（現在）のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//transitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);


			//描画先のRTVを設定する
			//指定した色で画面算体をクリアする

			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get() };
			commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

			//描画先とRTVとDSVの設定を行う
			//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvDescriptorHeap, desriptorSizeDSV, 0);
			commandList->OMSetRenderTargets(1, &rtVHandles[backBufferIndex], false, &dsvHandle);

			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
			commandList->ClearRenderTargetView(rtVHandles[backBufferIndex], clearColor, 0, nullptr);

			//指定した深度で画面をクリアする
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->RSSetViewports(1, &viewport);                            // Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);                      // Scissorを設定


			///
			/// 描画ここから 
			/// 

			//TODO:draw関数を作りたい
			//(x-min)/(max-min);


			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



			commandList->SetGraphicsRootSignature(rootSignature.Get());
			commandList->SetPipelineState(graphicsPipelineState.Get());                 // PSOを設定

			commandList->SetGraphicsRootConstantBufferView(4, directionalLightResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(5, cameraResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(6, pointLightResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(7, spotLightResource->GetGPUVirtualAddress());

			//DrawSprite(commandList, sprite, directionalLightResource, sprite->textureHandle);
			DrawSphere(commandList, sphere.get(), sphere->textureHandle);
			DrawSphere(commandList, pointLight.get(), sphere->textureHandle);



			//commandList->SetGraphicsRootSignature(rootSignatureForInstancing.Get());
			//commandList->SetPipelineState(graphicsPipelineStateForInstancing.Get());                 // PSOを設定


			//commandList->IASetVertexBuffers(0, 1, &plane->vertexBufferView);
			//commandList->SetGraphicsRootConstantBufferView(0, plane->materialResource->GetGPUVirtualAddress());
			//commandList->SetGraphicsRootConstantBufferView(1, plane->wvpResource->GetGPUVirtualAddress());
			//commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(plane->textureHandle));
			//commandList->SetGraphicsRootConstantBufferView(3, plane->useTextureResource->GetGPUVirtualAddress());
			//commandList->DrawInstanced(UINT(plane->vertices.size()), 1, 0, 0);

			commandList->IASetVertexBuffers(0, 1, &terrianModel->vertexBufferView);
			commandList->SetGraphicsRootConstantBufferView(0, terrianModel->materialResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, terrianModel->wvpResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(terrianModel->textureHandle));
			commandList->SetGraphicsRootConstantBufferView(3, terrianModel->useTextureResource->GetGPUVirtualAddress());
			commandList->DrawInstanced(UINT(terrianModel->vertices.size()), 1, 0, 0);

			///
			/// 描画ここまで
			/// 

			ImGui::Render();

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

			//画面に書く処理はすべて終わり，画面に移すので状態を遷移
			//今回はRenderTargetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			//コマンドリストの内容を確立させる。すべてのコマンドを積んでからcloseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			/// コマンドをキックする
			//GPUにコマンドリストの実行を行わせる
			Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
			//GPUとOSに画面の交換を行うように通知する
			swapChain->Present(1, 0);			//	画面が切り替わる

			/// GPUにSignalを送る
			//Fenceの値の更新
			fenceValue++;
			//GPUがここまでたどり着いたときに，Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence.Get(), fenceValue);

			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompleteValueの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue)
			{
				//指定したSignalにたどり着いていないので，たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベント待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator.Get(), nullptr);
			assert(SUCCEEDED(hr));


		}
	}

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DeleteTextures();

#ifdef _DEBUG
	debugController->Release();
#endif // _DEBUG
	CloseWindow(hwnd);




	CoUninitialize();

	return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して，アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
//
//void Log(const std::string& message)
//{
//	OutputDebugStringA(message.c_str());
//}
//
//
//std::wstring ConvertString(const std::string& str) {
//	if (str.empty()) {
//		return std::wstring();
//	}
//
//	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
//	if (sizeNeeded == 0) {
//		return std::wstring();
//	}
//	std::wstring result(sizeNeeded, 0);
//	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
//	return result;
//}
//
//std::string ConvertString(const std::wstring& str) {
//	if (str.empty()) {
//		return std::string();
//	}
//
//	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
//	if (sizeNeeded == 0) {
//		return std::string();
//	}
//	std::string result(sizeNeeded, 0);
//	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
//	return result;
//}

Microsoft::WRL::ComPtr<IDxcBlob> ComplieShader(const std::wstring& _filePath, const wchar_t* _profile, Microsoft::WRL::ComPtr<IDxcUtils>& _dxcUtils, Microsoft::WRL::ComPtr<IDxcCompiler3>& _dxcCompiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler>& _includeHandler)
{
	//hlslファイルを読み込む
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", _filePath, _profile)));
	//hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = _dxcUtils->LoadFile(_filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだ内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//Cmpileする
	LPCWSTR arguments[] = {
		_filePath.c_str(),      //コンパイル対象のhlslファイル名
		L"-E",L"main",          //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",_profile,         // shaderprofilerの設定
		L"-Zi",L"^Qembed_debug" // デバッグ用の情報を埋め込む
		L"-Od",                 // 最適化外しておく
		L"-Zpr",                // メモリレイアウトは行優先
	};
	//実際にshaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = _dxcCompiler->Compile(
		&shaderSourceBuffer,            // 読み込んだファイル
		arguments,			            // コンパイルオプション
		_countof(arguments),            // コンパイルオプションの数
		_includeHandler.Get(),	        // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)     // コンパイル結果
	);

	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Compile Succesed,path:{},profile:{}\n", _filePath, _profile)));
	////もう使わないリソースを解放
	//shaderSource->Release();
	//shaderResult->Release();
	//shaderError->Release();

	//実行用バイナリを返却
	return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, size_t _sizeInBytes)
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
	HRESULT hr = _device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
												  &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												  IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, int32_t _width, int32_t _height)
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
	HRESULT hr = _device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特別な設定は特になし。
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
		&depthClearValue, // Clear値の値
		IID_PPV_ARGS(resource.GetAddressOf())); // 作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));

	//resource->Release();

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;              //viewの情報を格納している場所(Discriptor)の束(配列)
	D3D12_DESCRIPTOR_HEAP_DESC descRiptorHeapDesc{};
	descRiptorHeapDesc.Type = _heapType;    //レンダーターゲットビュー(RTV)用
	descRiptorHeapDesc.NumDescriptors = _numDescriptors;                       //ダブルバッファ用に２つ。多くもかまわない
	descRiptorHeapDesc.Flags = _shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = _device->CreateDescriptorHeap(&descRiptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプターヒープが生成できなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

DirectX::ScratchImage LoadTexture(const std::string& _filePath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathw = ConvertString(_filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの生成
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す	
	return mipImage;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const DirectX::TexMetadata& _metadata)
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

	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置

	//resourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = _device->CreateCommittedResource(
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
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device.Get(), intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
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

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (_descriptorSize * _index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)

{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (_descriptorSize * _index);
	return handleGPU;
}

void DeleteTextures()
{
	textures.clear();
}

std::unique_ptr<ModelData> LoadObjFile(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize)
{
	 std::unique_ptr<ModelData>modelData = std::make_unique<ModelData>();				//構築するmodelData
	std::vector<Vector4> positions;		//位置
	std::vector<Vector3> normals;		//法線
	std::vector<Vector2> texcoords;		//テクスチャ座標
	std::string line;					//ファイルから読んだ1行を格納するもの

	std::ifstream file(_directoryPath + "/" + _filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/uV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);

				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.z *= -1.0f;
				normal.z *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;
				triangle[faceVertex] = { position,texcoord,normal };
			}
			modelData->vertices.push_back(triangle[2]);
			modelData->vertices.push_back(triangle[1]);
			modelData->vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			std::string mtlFilePath;
			s >> mtlFilePath;
			std::ifstream mtlFile(_directoryPath + "/" + mtlFilePath);
			assert(mtlFile.is_open());
			while (std::getline(mtlFile, line))
			{
				std::istringstream mtls(line);
				mtls >> identifier;

				if (identifier == "map_Kd")
				{
					std::string texturePath;
					mtls >> texturePath;

					modelData->textureHandlePath = _directoryPath + '/' + texturePath;
					modelData->textureHandle = LoadTexture(modelData->textureHandlePath, _device, _commandList, _srvDescriptorHeap, _srvSize);
				}
			}
		}
	}
	file.close();

	InitializeMeshData(_device, modelData.get());

	return modelData;
}

std::unique_ptr<ModelData> LoadObjFileWithAssimp(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize)
{
	std::unique_ptr<ModelData>modelData = std::make_unique<ModelData>();				//構築するmodelData

	Assimp::Importer importer;
	std::string filepath = _directoryPath + "/" + _filename;
	const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
	assert(scene->HasMeshes());// メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());						//法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0));			// TexcoordがないMeshは今回は非対応
		//ここからMeshの中身(Face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);				//三角形のみサポート
			//ここからFaceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				//aiProcess_MakeLeftHandedはz *= -1で、右手->左手に変換するので手動で対処
				vertex.position.z *= -1.0f;
				vertex.normal.z *= -1.0f;
				modelData->vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData->textureHandlePath = _directoryPath + "/" + textureFilePath.C_Str();
		}

	}
	InitializeMeshData(_device, modelData.get());


	return modelData;
}

std::unique_ptr<ModelData> LoadObjFileWithAssimp(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device)
{
	std::unique_ptr<ModelData>modelData = std::make_unique<ModelData>();				//構築するmodelData

	Assimp::Importer importer;
	std::string filepath = _directoryPath + "/" + _filename;
	const aiScene* scene = importer.ReadFile(filepath.c_str() ,aiProcess_FlipWindingOrder | aiProcess_FlipUVs); // 三角形の並びを逆に，UVのy軸反転
	assert(scene->HasMeshes());// メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());						//法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0));			// TexcoordがないMeshは今回は非対応
		//ここからMeshの中身(Face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);				//三角形のみサポート
			//ここからFaceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };		
				//aiProcess_MakeLeftHandedはz *= -1で、右手->左手に変換するので手動で対処
				vertex.position.z *= -1.0f;
				vertex.normal.z *= -1.0f;
				modelData->vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData->textureHandlePath = _directoryPath + "/" + textureFilePath.C_Str();
		}

	}
	modelData->rootNode = ReadNode(scene->mRootNode);

	InitializeMeshData(_device, modelData.get());


	return modelData;
}

std::unique_ptr<ModelData> LoadObjFile(const std::string& _directoryPath, const std::string& _filename, const Microsoft::WRL::ComPtr<ID3D12Device>& _device)
{
	std::unique_ptr<ModelData>modelData = std::make_unique<ModelData>();				//構築するmodelData
	std::vector<Vector4> positions;		//位置
	std::vector<Vector3> normals;		//法線
	std::vector<Vector2> texcoords;		//テクスチャ座標
	std::string line;					//ファイルから読んだ1行を格納するもの

	std::ifstream file(_directoryPath + "/" + _filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/uV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);

				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				//position.x *= -1.0f;
				position.z *= -1.0f;
				//normal.x *= -1.0f;
				normal.z *= -1.0f;
				//texcoord.x = 1.0f - texcoord.x;
				texcoord.y = 1.0f - texcoord.y;
				//VertexData vertex = { position, texcoord, normal };
				//modelData->vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}
			modelData->vertices.push_back(triangle[2]);
			modelData->vertices.push_back(triangle[1]);
			modelData->vertices.push_back(triangle[0]);
		}
	}
	file.close();

	return modelData;
}

Node ReadNode(aiNode* _node)
{
	Node result;
	aiMatrix4x4 aiLocalMatrix = _node->mTransformation;
	aiLocalMatrix.Transpose();

	for (uint32_t i = 0; i < 4; ++i) {
		for (uint32_t j = 0; j < 4; ++j) {
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}
	result.name = _node->mName.C_Str();
	result.chirdren.resize(_node->mNumChildren);
	for (uint32_t index = 0; index < _node->mNumChildren; ++index)
	{
		result.chirdren[index] = ReadNode(_node->mChildren[index]);
	}
	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(uint32_t _textureHandle)
{
	assert(textures.size() > _textureHandle);
	return textures[_textureHandle].srvHandlerGPU;
}

uint32_t LoadTexture(const std::string& _filePath, const  Microsoft::WRL::ComPtr<ID3D12Device>& _device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& _srvDescriptorHeap, uint32_t _srvSize)
{

	auto it = std::find_if(textures.begin(), textures.end(), [&](const auto& texture) {
		return texture.name == _filePath;
						   });

	if (it != textures.end())
	{
		return static_cast<uint32_t>(std::distance(textures.begin(), it));
	}

	textures.push_back(Texture());

	size_t index = textures.size() - 1;
	textures[index].name = _filePath;

	DirectX::ScratchImage mipImages = LoadTexture(_filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textures[index].resource = CreateTextureResource(_device, metadata);
	textures[index].intermediateResource = UploadTextureData(textures[index].resource, mipImages, _device, _commandList);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures[index].srvHandlerCPU = GetCPUDescriptorHandle(_srvDescriptorHeap, _srvSize, (uint32_t)index + 3);
	textures[index].srvHandlerGPU = GetGPUDescriptorHandle(_srvDescriptorHeap, _srvSize, (uint32_t)index + 3);
	_device->CreateShaderResourceView(textures[index].resource.Get(), &srvDesc, textures[index].srvHandlerCPU);

	return (uint32_t)index;
}

std::unique_ptr<Object>  MakeTriangleData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device)
{
	std::unique_ptr<Object> obj = std::make_unique<Object>();

	/// VertexResourcesを生成する
	obj->vertexResource = CreateBufferResource(_device, sizeof(VertexData) * 3);

	/// VertexBufferViewを作成する
	// 頂点バッファビューを作成する
	obj->vertexBufferView = { 0 };
	// リソースの実際のアドレスから
	obj->vertexBufferView.BufferLocation = obj->vertexResource->GetGPUVirtualAddress();
	// 構造付きバッファのサイズは頂点3つ分のサイズ
	obj->vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	obj->vertexBufferView.StrideInBytes = sizeof(VertexData);



	///色の変更
	obj->materialResource = CreateBufferResource(_device, sizeof(Vector4) * 3);
	obj->materialData = nullptr;
	obj->materialResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->materialData));
	obj->materialData->color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	obj->materialData->enabledLighthig = true;
	obj->materialData->uvTransform = MakeIdentity4x4();
	obj->materialData->shininess = 40.0f;

	//wvp用のリソースを作る。Matrix4x4 1つ分のサイズをサイズを用意する
	obj->wvpResource = CreateBufferResource(_device, sizeof(TransformationMatrix));
	//データを書き込む
	obj->transformMat = nullptr;
	//書き込むためのアドレスを取得
	obj->wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->transformMat));
	//単位行列を書き込んでおく
	obj->transformMat->World = MakeIdentity4x4();

	obj->useTextureResource = CreateBufferResource(_device, sizeof(float));
	obj->useTexture = nullptr;
	obj->useTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->useTexture));
	*obj->useTexture = 1.0f;

	/// Resourceにデータを書き込む
	// 頂点バッファーフォーマットが定義されて

	// 書き込むためのアクセス権限を取得
	obj->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->vertexData));
	// 左下
	obj->vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	obj->vertexData[0].texcoord = { 0.0f,1.0f };
	obj->vertexData[0].normal.x = obj->vertexData[0].position.x;
	obj->vertexData[0].normal.y = obj->vertexData[0].position.y;
	obj->vertexData[0].normal.z = obj->vertexData[0].position.z;
	// 上
	obj->vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	obj->vertexData[1].texcoord = { 0.5f,0.0f };
	obj->vertexData[1].normal.x = obj->vertexData[1].position.x;
	obj->vertexData[1].normal.y = obj->vertexData[1].position.y;
	obj->vertexData[1].normal.z = obj->vertexData[1].position.z;
	// 右下
	obj->vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	obj->vertexData[2].texcoord = { 1.0f,1.0f };
	obj->vertexData[2].normal.x = obj->vertexData[2].position.x;
	obj->vertexData[2].normal.y = obj->vertexData[2].position.y;
	obj->vertexData[2].normal.z = obj->vertexData[2].position.z;

	obj->vertexNum = 3;
	obj->textureHandle = -1;

	return obj;
}

std::unique_ptr<Object>  MakeSphereData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device)
{
	std::unique_ptr<Object> obj = std::make_unique<Object>();

	//分割数
	const uint32_t kSubdivision = 16;
	const uint32_t sphereVertexNum = kSubdivision * kSubdivision * 6;
	//sphere用の頂点リソ－スデータを作成
	obj->vertexResource = CreateBufferResource(_device, sizeof(VertexData) * sphereVertexNum);

	//頂点バッファビューを作成
	obj->vertexBufferView = { 0 };
	// リソースの先頭のアドレスから使う
	obj->vertexBufferView.BufferLocation = obj->vertexResource->GetGPUVirtualAddress();
	// 構造付きバッファのサイズは頂点6つ分のサイズ
	obj->vertexBufferView.SizeInBytes = sizeof(VertexData) * sphereVertexNum;
	// 1頂点あたりのサイズ
	obj->vertexBufferView.StrideInBytes = sizeof(VertexData);

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	obj->wvpResource = CreateBufferResource(_device, sizeof(TransformationMatrix));
	// データを書き込む
	obj->transformMat = nullptr;
	// 書き込むためのアドレスを取得
	obj->wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->transformMat));
	// 単位行列を書きこんでおく
	obj->transformMat->World = MakeIdentity4x4();


	obj->useTextureResource = CreateBufferResource(_device, sizeof(float));
	obj->useTexture = nullptr;
	obj->useTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->useTexture));
	*obj->useTexture = 1.0f;

	///色の変更
	obj->materialResource = CreateBufferResource(_device, sizeof(Material) * sphereVertexNum);
	obj->materialResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->materialData));
	obj->materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	obj->materialData->enabledLighthig = true;
	obj->materialData->uvTransform = MakeIdentity4x4();
	obj->materialData->shininess = 40.0f;

	obj->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->vertexData));

	//vertexの計算
	const float kLatEvery = (float)M_PI / (float)kSubdivision;          // 緯度分割１つ分の角度 θ
	const float kLonEvery = (float)M_PI * 2.0 / (float)kSubdivision;    // 経度分割１つ分の角度 φ

	//緯度の方向に分割   -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++)
	{
		float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;         // 現在の緯度

		// 経度の方向に分割   0 ~ π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++)
		{
			uint32_t startIndex = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;                           // 現在の経度

			//a
			obj->vertexData[startIndex].position.x = std::cosf(lat) * std::cosf(lon);
			obj->vertexData[startIndex].position.y = std::sinf(lat);
			obj->vertexData[startIndex].position.z = std::cosf(lat) * std::sinf(lon);
			obj->vertexData[startIndex].position.w = 1.0f;
			obj->vertexData[startIndex].texcoord.x = float(lonIndex) / float(kSubdivision);
			obj->vertexData[startIndex].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			obj->vertexData[startIndex].normal.x = obj->vertexData[startIndex].position.x;
			obj->vertexData[startIndex].normal.y = obj->vertexData[startIndex].position.y;
			obj->vertexData[startIndex].normal.z = obj->vertexData[startIndex].position.z;
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex++].normal);

			//b
			obj->vertexData[startIndex].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			obj->vertexData[startIndex].position.y = std::sinf(lat + kLatEvery);
			obj->vertexData[startIndex].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			obj->vertexData[startIndex].position.w = 1.0f;
			obj->vertexData[startIndex].texcoord.x = float(lonIndex) / float(kSubdivision);
			obj->vertexData[startIndex].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			obj->vertexData[startIndex].normal.x = obj->vertexData[startIndex].position.x;
			obj->vertexData[startIndex].normal.y = obj->vertexData[startIndex].position.y;
			obj->vertexData[startIndex].normal.z = obj->vertexData[startIndex].position.z;
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex++].normal);

			//c
			obj->vertexData[startIndex].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			obj->vertexData[startIndex].position.y = std::sinf(lat);
			obj->vertexData[startIndex].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			obj->vertexData[startIndex].position.w = 1.0f;
			obj->vertexData[startIndex].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			obj->vertexData[startIndex].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			obj->vertexData[startIndex].normal.x = obj->vertexData[startIndex].position.x;
			obj->vertexData[startIndex].normal.y = obj->vertexData[startIndex].position.y;
			obj->vertexData[startIndex].normal.z = obj->vertexData[startIndex].position.z;
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex++].normal);

			//bコピー
			obj->vertexData[startIndex] = obj->vertexData[startIndex - 2];
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex++].normal);

			//d
			obj->vertexData[startIndex].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery);
			obj->vertexData[startIndex].position.y = std::sinf(lat + kLatEvery);
			obj->vertexData[startIndex].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery);
			obj->vertexData[startIndex].position.w = 1.0f;
			obj->vertexData[startIndex].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			obj->vertexData[startIndex].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			obj->vertexData[startIndex].normal.x = obj->vertexData[startIndex].position.x;
			obj->vertexData[startIndex].normal.y = obj->vertexData[startIndex].position.y;
			obj->vertexData[startIndex].normal.z = obj->vertexData[startIndex].position.z;
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex++].normal);

			//cコピー
			obj->vertexData[startIndex] = obj->vertexData[startIndex - 3];
			obj->vertexData[startIndex].normal = Normalize(obj->vertexData[startIndex].normal);
		};
	}
	obj->vertexNum = sphereVertexNum;
	obj->textureHandle = 0;

	return obj;
}

std::unique_ptr<Object>   MakeSpriteData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device)
{
	std::unique_ptr<Object> obj = std::make_unique<Object>();


	//sprite用の頂点リソ－スデータを作成
	obj->vertexResource = CreateBufferResource(_device, sizeof(VertexData) * 6);

	// リソースの先頭のアドレスから使う
	obj->vertexBufferView.BufferLocation = obj->vertexResource->GetGPUVirtualAddress();
	// 構造付きバッファのサイズは頂点6つ分のサイズ
	obj->vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	obj->vertexBufferView.StrideInBytes = sizeof(VertexData);


	obj->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->vertexData));

	// 1枚目の三角形
	obj->vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	obj->vertexData[0].texcoord = { 0.0f, 1.0f };
	obj->vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	obj->vertexData[1].texcoord = { 0.0f, 0.0f };
	obj->vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	obj->vertexData[2].texcoord = { 1.0f, 1.0f };
	obj->vertexData[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	obj->vertexData[3].texcoord = { 1.0f, 0.0f };

	obj->vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	obj->vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	obj->vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	obj->vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	obj->indexResource = CreateBufferResource(_device, sizeof(uint32_t) * 6);
	obj->indexBufferView.BufferLocation = obj->indexResource->GetGPUVirtualAddress();
	obj->indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	obj->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	obj->indexData = nullptr;
	obj->indexResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->indexData));

	obj->indexData[0] = 0;
	obj->indexData[1] = 1;
	obj->indexData[2] = 2;
	obj->indexData[3] = 1;
	obj->indexData[4] = 3;
	obj->indexData[5] = 2;

	obj->materialResource = CreateBufferResource(_device, sizeof(Material));

	obj->materialData = new Material;
	obj->materialResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->materialData));
	obj->materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	obj->materialData->enabledLighthig = false;
	obj->materialData->uvTransform = MakeIdentity4x4();
	obj->materialData->shininess = 40.0f;


	obj->useTextureResource = CreateBufferResource(_device, sizeof(float));
	obj->useTexture = nullptr;
	obj->useTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->useTexture));
	*obj->useTexture = 1.0f;

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	obj->wvpResource = CreateBufferResource(_device, sizeof(TransformationMatrix));
	// データを書き込む
	obj->transformMat = nullptr;
	// 書き込むためのアドレスを取得
	obj->wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&obj->transformMat));
	// 単位行列を書きこんでおく
	obj->transformMat->World = MakeIdentity4x4();
	obj->transformMat->WVP = MakeIdentity4x4();

	obj->textureHandle = 0;

	return obj;
}

std::unique_ptr<ModelData> MakeModelData(Microsoft::WRL::ComPtr<ID3D12Device>& _device, const std::string& _directoryPath, const std::string& _filename)
{
	std::unique_ptr<ModelData> model = std::make_unique<ModelData>();
	//モデル読み込み
	model = LoadObjFile(_directoryPath, _filename, _device);
	InitializeMeshData(_device, model.get());

	return model;
}

void InitializeMeshData(const Microsoft::WRL::ComPtr<ID3D12Device>& _device, ModelData* _model)
{
	//頂点リソースを作る
	_model->vertexResource = CreateBufferResource(_device, sizeof(VertexData) * _model->vertices.size());
	//頂点バッファビューを作成する
	_model->vertexBufferView.BufferLocation = _model->vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	_model->vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * _model->vertices.size());//使用するリソースのサイズは頂点のサイズ
	_model->vertexBufferView.StrideInBytes = sizeof(VertexData);//1頂点あたりのサイズ

	//頂点リソースにデータを書き込む
	_model->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&_model->vertexData)); //書き込むためのアドレスを取得
	std::memcpy(_model->vertexData, _model->vertices.data(), sizeof(VertexData) * _model->vertices.size());//頂点データをリソースにコピー

	_model->materialResource = CreateBufferResource(_device, sizeof(Material));
	_model->materialResource->Map(0, nullptr, reinterpret_cast<void**>(&_model->materialData));
	_model->materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	_model->materialData->enabledLighthig = true;
	_model->materialData->uvTransform = MakeIdentity4x4();
	_model->materialData->shininess = 40.0f;

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	_model->wvpResource = CreateBufferResource(_device, sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	_model->wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&_model->transformMat));
	// 単位行列を書きこんでおく
	_model->transformMat->World = MakeIdentity4x4();

	_model->useTextureResource = CreateBufferResource(_device, sizeof(float));
	_model->useTexture = nullptr;
	_model->useTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&_model->useTexture));
	*_model->useTexture = 1.0f;
}


TransformationMatrix CalculateSpriteWVPMat(const stTransform& _transform)
{
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);

	TransformationMatrix transMat;
	transMat.World = MakeAffineMatrix(_transform.scale, _transform.rotate, _transform.translate);
	transMat.WVP = Multiply(transMat.World, Multiply(viewMatrix, projectionMatrix));

	return TransformationMatrix(transMat);
}

TransformationMatrix CalculateObjectWVPMat(const stTransform& _transform, const Matrix4x4& _VPmat)
{
	TransformationMatrix transMat;
	transMat.World = MakeAffineMatrix(_transform.scale, _transform.rotate, _transform.translate);
	transMat.WVP = transMat.World * _VPmat;
	transMat.worldInverseTranspose = Transpose(Inverse(transMat.World));
	return TransformationMatrix(transMat);
}

void DrawTriangle(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, Microsoft::WRL::ComPtr<ID3D12Resource> _light, uint32_t _textureHandle)
{
	_commandList->IASetVertexBuffers(0, 1, &_obj->vertexBufferView);

	_commandList->SetGraphicsRootConstantBufferView(0, _obj->materialResource->GetGPUVirtualAddress());
	_commandList->SetGraphicsRootConstantBufferView(1, _obj->wvpResource->GetGPUVirtualAddress());
	if (_textureHandle == -1)
	{
		*_obj->useTexture = 0.0f;
		_commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(0));
	}
	else
	{
		*_obj->useTexture = 1.0f;
		_commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(_obj->textureHandle));
	}
	_commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(_textureHandle));
	_commandList->SetGraphicsRootConstantBufferView(3, _obj->useTextureResource->GetGPUVirtualAddress());

	_commandList->DrawInstanced(3, 1, 0, 0);
}

void DrawSprite(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, Microsoft::WRL::ComPtr<ID3D12Resource> _light, uint32_t _textureHandle)
{
	_commandList->IASetVertexBuffers(0, 1, &_obj->vertexBufferView);
	_commandList->IASetIndexBuffer(&_obj->indexBufferView);

	_commandList->SetGraphicsRootConstantBufferView(0, _obj->materialResource->GetGPUVirtualAddress());
	_commandList->SetGraphicsRootConstantBufferView(1, _obj->wvpResource->GetGPUVirtualAddress());

	_commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(_obj->textureHandle));
	_commandList->SetGraphicsRootConstantBufferView(3, _obj->useTextureResource->GetGPUVirtualAddress());

	_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void DrawSphere(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList, Object* _obj, uint32_t _textureHandle)
{
	_commandList->IASetVertexBuffers(0, 1, &_obj->vertexBufferView);

	_commandList->SetGraphicsRootConstantBufferView(0, _obj->materialResource->GetGPUVirtualAddress());
	_commandList->SetGraphicsRootConstantBufferView(1, _obj->wvpResource->GetGPUVirtualAddress());
	_commandList->SetGraphicsRootDescriptorTable(2, GetTextureHandle(_obj->textureHandle));
	_commandList->SetGraphicsRootConstantBufferView(3, _obj->useTextureResource->GetGPUVirtualAddress());

	_commandList->DrawInstanced(_obj->vertexNum, 1, 0, 0);
}

Particle MakeNewParticle(std::mt19937& _randomEngine, const Emitter& _emitter)
{

	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	Vector3 randomTranslate{ distribution(_randomEngine), distribution(_randomEngine), distribution(_randomEngine) };
	particle.transform.translate = _emitter.transform.translate + randomTranslate;

	particle.velocity = { distribution(_randomEngine), distribution(_randomEngine), distribution(_randomEngine) };



	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	particle.color = { distColor(_randomEngine), distColor(_randomEngine), distColor(_randomEngine),1.0f };

	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	particle.lifeTime = distTime(_randomEngine);
	particle.currentTime = 0;


	return particle;
}

std::list<Particle> Emit(const Emitter& _emitter, std::mt19937& _randomEngine)
{
	std::list<Particle> particles;
	for (uint32_t count = 0; count < _emitter.count; count++)
	{
		particles.push_back(MakeNewParticle(_randomEngine, _emitter));
	}
	return particles;
}

void SetBlendMode(BlendMode _blendMode, D3D12_GRAPHICS_PIPELINE_STATE_DESC& _graphicsPipelineStateDesc)
{
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	switch (_blendMode)
	{
	case BlendMode::kBlendModeNormal:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BlendMode::kBlendModeAdd:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeSub:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case BlendMode::kBlendModeMultiply:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case BlendMode::kBlendModeScreen:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	default:
		break;
	}

	_graphicsPipelineStateDesc.BlendState = blendDesc;                                                               // BlendState


}

TransformationMatrix CalculateParticleWVPMat(const stTransform& _transform, const Matrix4x4& _cameraMatrix, const Matrix4x4& _VPmat, bool useBillborad)
{
	TransformationMatrix transMat;


	Matrix4x4 billboardatrix;
	if (useBillborad)
	{
		billboardatrix = _cameraMatrix;
		billboardatrix.m[3][0] = 0;
		billboardatrix.m[3][1] = 0;
		billboardatrix.m[3][2] = 0;
	}
	else
		billboardatrix = MakeIdentity4x4();

	Matrix4x4 scaleMatrix = MakeScaleMatrix(_transform.scale);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(_transform.translate);

	transMat.World = scaleMatrix * billboardatrix * translateMatrix;
	transMat.WVP = transMat.World * _VPmat;

	return TransformationMatrix(transMat);
}
