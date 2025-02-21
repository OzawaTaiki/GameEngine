#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Debug/Debug.h>
#include <Utility/ConvertString/ConvertString.h>
#include <cassert>

PSOManager* PSOManager::GetInstance()

{
    static PSOManager instance;
    return &instance;
}

void PSOManager::Initialize()
{
    dxCommon_ = DXCommon::GetInstance();
    assert(dxCommon_);

    HRESULT hr = S_FALSE;

    // 各変数を生成
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));
    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));


    CreatePSOForModel           (PSOFlags::Type_Model           | PSOFlags::Blend_Normal    | PSOFlags::Cull_Back);
    //CreatePSOForAnimationModel  (PSOFlags::Type_AnimationModel  | PSOFlags::Blend_Normal    | PSOFlags::Cull_Back);
    CreatePSOForSprite          (PSOFlags::Type_Sprite          | PSOFlags::Blend_Normal    | PSOFlags::Cull_Back);
    CreatePSOForLineDrawer      (PSOFlags::Type_LineDrawer      | PSOFlags::Blend_Normal    | PSOFlags::Cull_None);
    CreatePSOForParticle        (PSOFlags::Type_Particle        | PSOFlags::Blend_Add       | PSOFlags::Cull_Back);
    CreatePSOForOffScreen       ();
    CreatePSOForShadowMap();
}

std::optional<ID3D12PipelineState*> PSOManager::GetPipeLineStateObject(PSOFlags _flag)
{
    size_t index = static_cast<size_t>(_flag);
    // 要素があるか確認
    auto it = graphicsPipelineStates_.find(index);
    // あったらそれを返す
    if (it != graphicsPipelineStates_.end())
        return graphicsPipelineStates_[index].Get();
    // なかったらnullを返す
    else
        return std::nullopt;
}

std::optional<ID3D12RootSignature*> PSOManager::GetRootSignature(PSOFlags _flag)
{
    size_t type = GetType(_flag);
    // 要素があるか確認
    auto it = rootSignatures_.find(type);
    // あったらそれを返す
    if (it != rootSignatures_.end())
        return rootSignatures_[type].Get();
    // なかったらnullを返す
    else
        return std::nullopt;
}

void PSOManager::SetPipeLineStateObject(PSOFlags _flag)
{
    size_t index = static_cast<size_t>(_flag);
    // 要素があるか確認
    auto it = graphicsPipelineStates_.find(index);
    if (it == graphicsPipelineStates_.end())
    {
        assert(false && "PSOが見つかりません");
    }

    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineStates_[index].Get());
}

void PSOManager::SetRootSignature(PSOFlags _flag)
{
    size_t type = GetType(_flag);
    // 要素があるか確認
    auto it = rootSignatures_.find(type);
    if (it == rootSignatures_.end())
    {
        assert(false && "RootSignatureが見つかりません");
    }

    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignatures_[type].Get());

}


Microsoft::WRL::ComPtr<IDxcBlob> PSOManager::ComplieShader(
    const std::wstring& _filePath,
    const wchar_t* _profile,
    const std::wstring& _entryFuncName,
    const std::wstring& _dirPath)
{

    std::wstring fullpath = _dirPath + _filePath;

    //hlslファイルを読み込む
 	//これからシェーダーをコンパイルする旨をログに出す
    Debug::Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", fullpath, _profile)));
 	//hlslファイルを読む
 	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils_->LoadFile(fullpath.c_str(), nullptr, &shaderSource);
 	//読めなかったら止める
 	assert(SUCCEEDED(hr));
 	//読み込んだ内容を設定する
 	DxcBuffer shaderSourceBuffer;
 	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
 	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
 	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

 	//Compileする
 	LPCWSTR arguments[] = {
 		_filePath.c_str(),      //コンパイル対象のhlslファイル名
 		L"-E",_entryFuncName.c_str(),          //エントリーポイントの指定。基本的にmain以外にはしない
 		L"-T",_profile,         // shaderprofilerの設定
 		L"-Zi",L"^Qembed_debug" // デバッグ用の情報を埋め込む
 		L"-Od",                 // 最適化外しておく
 		L"-Zpr",                // メモリレイアウトは行優先
 	};
 	//実際にshaderをコンパイルする
 	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
 	hr = dxcCompiler_->Compile(
 		&shaderSourceBuffer,            // 読み込んだファイル
 		arguments,			            // コンパイルオプション
 		_countof(arguments),            // コンパイルオプションの数
 		includeHandler_.Get(),	        // includeが含まれた諸々
 		IID_PPV_ARGS(&shaderResult)     // コンパイル結果
 	);

 	assert(SUCCEEDED(hr));

 	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
 	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
 	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
 	{
        Debug::Log(shaderError->GetStringPointer());
 		assert(false);
 	}

 	//コンパイル結果から実行用のバイナリ部分を取得
 	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
 	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
 	assert(SUCCEEDED(hr));
 	//成功したログを出す
    Debug::Log(ConvertString(std::format(L"Compile Succesed,path:{},profile:{}\n", fullpath, _profile)));

 	//実行用バイナリを返却
 	return shaderBlob;
}

void PSOManager::CreatePSOForModel(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(_flags);

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う


    // シャドウマップ用のサンプラ
    staticSamplers[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 比較用フィルタ
    staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER; // 境界外を無効化
    staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // 影の外は光が当たる
    staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 深度比較
    staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[1].ShaderRegister = 1; // s1
    staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    D3D12_DESCRIPTOR_RANGE shadowMapRange[1] = {};
    shadowMapRange[0].BaseShaderRegister = 1;  // t1 にバインド
    shadowMapRange[0].NumDescriptors = 1;
    shadowMapRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    shadowMapRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[7] = {};

    //カメラ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // transform
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       // VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    //マテリアル
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
    rootParameters[2].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    // 色
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 2;

    // テクスチャ
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[4].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[4].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

    // LightGroup
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 3;

    // シャドウマップ
    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[6].DescriptorTable.pDescriptorRanges = shadowMapRange;
    rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(shadowMapRange);

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
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

#pragma region InputLayout
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
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Object3d.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Object3d.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(_flags);
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(_flags);

    //三角形を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;


    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForAnimationModel(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(_flags);

#pragma region Sampler
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
#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;//０から始まる
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    D3D12_DESCRIPTOR_RANGE descriptorRangeForWell[1] = {};
    descriptorRangeForWell[0].BaseShaderRegister = 0;//０から始まる
    descriptorRangeForWell[0].NumDescriptors = 1;//数は１つ
    descriptorRangeForWell[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRangeForWell[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[7] = {};

    //カメラ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // transform
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       // VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    //マテリアル
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
    rootParameters[2].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    // 色
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 2;

    // テクスチャ
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[4].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[4].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

    // LightGroup
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 3;

    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;			//pixelShaderで使う
    rootParameters[6].DescriptorTable.pDescriptorRanges = descriptorRangeForWell;		//tableの中身の配列を指定
    rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForWell);//tableで利用する数

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
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

#pragma region InputLayout
    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] = {};
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


    inputElementDescs[3].SemanticName = "WEIGHT";
    inputElementDescs[3].SemanticIndex = 0;
    inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[3].InputSlot = 1;
    inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[4].SemanticName = "INDEX";
    inputElementDescs[4].SemanticIndex = 0;
    inputElementDescs[4].Format = DXGI_FORMAT_R32G32B32A32_SINT;
    inputElementDescs[4].InputSlot = 1;
    inputElementDescs[4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"SkinningObject3d.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Object3d.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};

    blendDesc = GetBlendDesc(_flags);
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};

    rasterizerDesc = GetRasterizerDesc(_flags);

#pragma endregion


    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                      // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;

    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForSprite(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(_flags);

#pragma region Sampler
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
#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;//０から始まる
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // matrix
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // color
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 1;

    // テクスチャ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数



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
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = false;
#pragma endregion

#pragma region InputLayout
    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ComplieShader(L"Sprite.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = ComplieShader(L"Sprite.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(_flags);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(_flags);

#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForLineDrawer(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(_flags);

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_ANISOTROPIC; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 比較しない
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // PixelShaderで使う
#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[1] = {};

    // matrix
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStencilDesc.StencilEnable = false;
#pragma endregion

#pragma region InputLayout
    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ComplieShader(L"LineDrawer.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = ComplieShader(L"LineDrawer.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc = GetBlendDesc(_flags);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(_flags);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.BlendState = blendDesc;                                                               // BlendState
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForParticle(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(_flags);

#pragma region Sampler
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
#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRangeTrans[1] = {};
    descriptorRangeTrans[0].BaseShaderRegister = 0;//０から始まる
    descriptorRangeTrans[0].NumDescriptors = 1;//数は１つ
    descriptorRangeTrans[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRangeTrans[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    D3D12_DESCRIPTOR_RANGE descriptorRangeTexture[1] = {};
    descriptorRangeTexture[0].BaseShaderRegister = 0;//０から始まる
    descriptorRangeTexture[0].NumDescriptors = 1;//数は１つ
    descriptorRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    //カメラ   gViewProjection
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // transform    ParticleForGPU
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeTrans;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeTrans);

    // テクスチャ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeTexture;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeTexture);


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
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    //書き込みしない
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

#pragma region InputLayout
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
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(_flags);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(_flags);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                   // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForOffScreen()
{
    HRESULT hr = S_FALSE;

    size_t type = GetType(PSOFlags::Type_OffScreen | PSOFlags::Cull_None | PSOFlags::Blend_Normal);

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_ANISOTROPIC; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 比較しない
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // PixelShaderで使う
#pragma endregion

#pragma region RootSignature

    ///// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;//０から始まる
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[1] = {};

    // テクスチャ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);;

    ////シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
#pragma endregion

#pragma region InputLayout
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ComplieShader(L"FullScreen.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = ComplieShader(L"FullScreen.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc = GetBlendDesc(PSOFlags::Blend_Normal);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(PSOFlags::Cull_None);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.BlendState = blendDesc;                                                               // BlendState
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[type] = pipelineState;

}

void PSOManager::CreatePSOForShadowMap()
{
    HRESULT hr = S_FALSE;

    uint32_t type = GetType(PSOFlags::Type_ShadowMap);

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

    // シャドウマップ用のサンプラ
    staticSamplers[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 比較用フィルタ
    staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER; // 境界外を無効化
    staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // 影の外は光が当たる
    staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS; // 深度比較
    staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[1].ShaderRegister = 1; // s1
    staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


#pragma endregion

#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[6] = {};

    //カメラ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // transform
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       // VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    //マテリアル
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;           // CBVを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;        // PixelShaderで使う
    rootParameters[2].Descriptor.ShaderRegister = 1;                           // レジスタ番号0を使う

    // 色
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 2;

    // テクスチャ
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[4].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[4].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数

    // LightGroup
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 3;


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
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[type]));
    assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DepthStencilState
    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    depthStencilDesc.StencilEnable = false;
#pragma endregion

#pragma region InputLayout
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
#pragma endregion

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"ShadowMap.hlsl", L"vs_6_0",L"ShadowMapVS");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"ShadowMap.hlsl", L"ps_6_0", L"ShadowMapPS");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(PSOFlags::Blend_Normal);

#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(PSOFlags::Cull_None);

    //三角形を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                                     // RasterizerState
    // 追加の DRTV の情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    // どのように画面に色を打ち込むかの設定 (気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;


    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<size_t>(type)] = pipelineState;

}

D3D12_BLEND_DESC PSOManager::GetBlendDesc(PSOFlags _flag)
{
    D3D12_BLEND_DESC blendDesc{};

    PSOFlags mode = GetBlendMode(_flag);

    switch (mode)
    {
    case PSOFlags::Blend_Normal:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;
    case PSOFlags::Blend_Add:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case PSOFlags::Blend_Sub:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case PSOFlags::Blend_Multiply:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        break;
    case PSOFlags::Blend_Screen:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    default:
        break;
    }

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    return blendDesc;
}

D3D12_RASTERIZER_DESC PSOManager::GetRasterizerDesc(PSOFlags _flag)
{
    if (!IsSingleBitSetInMask(_flag, CullMask))
    {
        assert("Cullが複数設定されています" && false);
    }

    D3D12_RASTERIZER_DESC rasterizerDesc{};

    if (HasFlag(_flag, PSOFlags::Cull_None))
    {
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    }
    else if (HasFlag(_flag, PSOFlags::Cull_Back))
    {
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    }
    else if (HasFlag(_flag, PSOFlags::Cull_Front))
    {
        rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    }

    return rasterizerDesc;
}


size_t PSOManager::GetType(PSOFlags _flag)
{
    if(!IsSingleBitSetInMask(_flag,TypeMask))
    {
        assert("Typeが複数設定されています" && false);
    }

    if (HasFlag(_flag, PSOFlags::Type_Model))
    {
        return static_cast<size_t>(PSOFlags::Type_Model);
    }
    else if (HasFlag(_flag, PSOFlags::Type_AnimationModel))
    {
        return static_cast<size_t>(PSOFlags::Type_AnimationModel);
    }
    else if (HasFlag(_flag, PSOFlags::Type_Sprite))
    {
        return static_cast<size_t>(PSOFlags::Type_Sprite);
    }
    else if (HasFlag(_flag, PSOFlags::Type_LineDrawer))
    {
        return static_cast<size_t>(PSOFlags::Type_LineDrawer);
    }
    else if (HasFlag(_flag, PSOFlags::Type_Particle))
    {
        return static_cast<size_t>(PSOFlags::Type_Particle);
    }
    else if (HasFlag(_flag, PSOFlags::Type_OffScreen))
    {
        return static_cast<size_t>(PSOFlags::Type_OffScreen);
    }
    else if (HasFlag(_flag, PSOFlags::Type_ShadowMap))
    {
        return static_cast<size_t>(PSOFlags::Type_ShadowMap);
    }


    assert("Typeが設定されていません" && false);
    return 0;

}

PSOFlags PSOManager::GetBlendMode(PSOFlags _flag)
{
    if (!IsSingleBitSetInMask(_flag, BlendMask))
    {
        assert("BlendModeが複数設定されています" && false);
        return PSOFlags::Blend_Normal;
    }

    if (HasFlag(_flag, PSOFlags::Blend_Normal))
    {
        return  PSOFlags::Blend_Normal;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Add))
    {
        return  PSOFlags::Blend_Add;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Sub))
    {
        return  PSOFlags::Blend_Sub;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Multiply))
    {
        return  PSOFlags::Blend_Multiply;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Screen))
    {
        return  PSOFlags::Blend_Screen;
    }

    // デフォルトはNormal
    return  PSOFlags::Blend_Normal;
}

PSOFlags SetBlendMode(PSOFlags _flag, PSOFlags _mode)
{
    if (!IsSingleBitSetInMask(_flag, BlendMask))
    {
        assert(false);
    }

    PSOFlags flag = _flag;

    flag &= ~(PSOFlags::Blend_Normal | PSOFlags::Blend_Add | PSOFlags::Blend_Sub | PSOFlags::Blend_Multiply | PSOFlags::Blend_Screen);
    flag |= _mode;

    return flag;
}

PSOFlags GetBlendMode(BlendMode _mode)
{
    switch (_mode)
    {
    case BlendMode::Normal:
        return PSOFlags::Blend_Normal;
    case BlendMode::Add:
        return PSOFlags::Blend_Add;
    case BlendMode::Sub:
        return PSOFlags::Blend_Sub;
    case BlendMode::Multiply:
        return PSOFlags::Blend_Multiply;
    case BlendMode::Screen:
        return PSOFlags::Blend_Screen;
    default:
        return PSOFlags::Blend_Normal;
    }
}

BlendMode GetBlendMode(PSOFlags _flag)
{
    if (!IsSingleBitSetInMask(_flag, BlendMask))
    {
        assert(false);
    }
    if (HasFlag(_flag, PSOFlags::Blend_Normal))
    {
        return BlendMode::Normal;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Add))
    {
        return BlendMode::Add;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Sub))
    {
        return BlendMode::Sub;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Multiply))
    {
        return BlendMode::Multiply;
    }
    else if (HasFlag(_flag, PSOFlags::Blend_Screen))
    {
        return BlendMode::Screen;
    }
    return BlendMode::Normal;
}


bool IsValidPSOFlags(PSOFlags _flags)
{
    // Type, Blend, Cullのビットがそれぞれ一つだけセットされているか確認
    bool isTypeValid = IsSingleBitSetInMask(_flags, TypeMask);
    bool isBlendValid = IsSingleBitSetInMask(_flags, BlendMask);
    bool isCullValid = IsSingleBitSetInMask(_flags, CullMask);

    return isTypeValid && isBlendValid && isCullValid;
}

bool IsSingleBitSetInMask(PSOFlags _flags, PSOFlags _mask)
{
    size_t maskedFlag = static_cast<size_t>(_flags & _mask);
    return (maskedFlag != 0) && ((maskedFlag & (maskedFlag - 1)) == 0);
}

bool HasFlag(PSOFlags _flag, PSOFlags _checkFlag)
{
    return (_flag & _checkFlag) == _checkFlag;
}

