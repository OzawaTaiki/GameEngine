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

    CreateDefaultPSOs();

}

std::optional<ID3D12PipelineState*> PSOManager::GetPipeLineStateObject(PSOFlags _flag)
{
    uint64_t index = static_cast<uint64_t>(_flag);
    // 要素があるか確認
    auto it = graphicsPipelineStates_.find(index);
    // あったらそれを返す
    if (it != graphicsPipelineStates_.end())
        return graphicsPipelineStates_[index].Get();
    // なかったらnullを返す
    else
    {
        PSOFlags::Type type = _flag.GetType();
        uint64_t index = static_cast<uint64_t>(_flag);

        switch (type)
        {
        case PSOFlags::Type::None:
            assert(false && "PSOのTypeが設定されていません");
            return std::nullopt;
            break;
        case PSOFlags::Type::Model:
            CreatePSOForModel(_flag);
            return graphicsPipelineStates_[index].Get();
            break;
        case PSOFlags::Type::Sprite:
            CreatePSOForSprite(_flag);
            return graphicsPipelineStates_[index].Get();
            break;
        case PSOFlags::Type::LineDrawer:
            CreatePSOForLineDrawer(_flag);
            return graphicsPipelineStates_[index].Get();
            break;
        case PSOFlags::Type::Particle:
            CreatePSOForParticle(_flag);
            return graphicsPipelineStates_[index].Get();
            break;
        default:
            break;
        }

        return std::nullopt;
    }
}

std::optional<ID3D12RootSignature*> PSOManager::GetRootSignature(PSOFlags _flag)
{
    uint64_t type = _flag.GetTypeValue();
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
    uint64_t index = _flag;
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
    uint64_t type = _flag.GetTypeValue();
    // 要素があるか確認
    auto it = rootSignatures_.find(type);
    if (it == rootSignatures_.end())
    {
        assert(false && "RootSignatureが見つかりません");
    }

    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignatures_[type].Get());

}

void PSOManager::SetPSOForPostEffect(const std::string& _name)
{
    // 要素があるか確認
    auto it = postEffectPipelineStates_.find(_name);
    if (it == postEffectPipelineStates_.end())
    {
        assert(false && "PostEffectPSOが見つかりません");
    }

    dxCommon_->GetCommandList()->SetPipelineState(postEffectPipelineStates_[_name].Get());
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignatures_[static_cast<uint64_t>(PSOFlags::Type::OffScreen)].Get());
}

void PSOManager::SetRegisterPSO(const std::string& _name)
{
    auto it = registerPSO_.find(_name);
    if (it == registerPSO_.end())
        assert(false && "PSOがみつかりません");

    dxCommon_->GetCommandList()->SetPipelineState(registerPSO_[_name].Get());
}

void PSOManager::SetRegisterRootSignature(const std::string& _name)
{
    auto it = regiterRootSignature_.find(_name);
    if (it == regiterRootSignature_.end())
        assert(false && "RootSignatureがみつかりません");
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(regiterRootSignature_[_name].Get());
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

void PSOManager::CreatePSOForPostEffect(const std::string& _name,
    const std::wstring& _psFileName,
    const std::wstring& _entryFuncName,
    const std::wstring& _dirPath)
{
    PSOFlags flag = PSOFlags::Type::OffScreen | PSOFlags::BlendMode::Normal |
        PSOFlags::CullMode::None |
        PSOFlags::DepthMode::Disable;

    HRESULT hr = S_FALSE;

    uint64_t rootSignatureType = flag.GetTypeValue();
#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
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
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

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
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[rootSignatureType]));
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
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = ComplieShader(_psFileName, L"ps_6_0", _entryFuncName, _dirPath);
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc = GetBlendDesc(flag);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[rootSignatureType].Get();                                         // RootSignature
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

    postEffectPipelineStates_[_name] = pipelineState;

}

void PSOManager::RegisterPSO(const std::string& _name, ID3D12PipelineState* _pso)
{
    registerPSO_[_name] = _pso;
}

void PSOManager::RegisterRootSignature(const std::string& _name, ID3D12RootSignature* _rs)
{
    regiterRootSignature_[_name] = _rs;
}

void PSOManager::CreatePSOForSkyBox()
{

    PSOFlags flag = PSOFlags::Type::SkyBox | PSOFlags::BlendMode::Normal | PSOFlags::CullMode::Back | PSOFlags::DepthMode::Comb_mAll_fLessEqual;

    HRESULT hr = S_FALSE;

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
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;//数は１つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//ofsetを自動計算


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // transform
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // camera
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 1;

    // color
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].Descriptor.ShaderRegister = 2;

    // テクスチャ
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableで使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//pixelShaderで使う
    rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRange;		//tableの中身の配列を指定
    rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数



    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    uint64_t type = flag.GetTypeValue();

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
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //比較関数はLessEqeul つまり近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion


#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"SkyBox.hlsl", L"vs_6_0",L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"SkyBox.hlsl", L"ps_6_0", L"PSmain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc = GetBlendDesc(flag);
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);

    //三角形を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
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


    graphicsPipelineStates_[static_cast<uint64_t>(flag)] = pipelineState;
}

void PSOManager::CreateDefaultPSOs()
{

    CreatePSOForModel(PSOFlags::ForNormalModel());
    CreatePSOForModel(PSOFlags::ForAlphaModel());
    CreatePSOForSprite(PSOFlags::ForSprite());
    CreatePSOForLineDrawer(PSOFlags::ForLineDrawer());
    CreatePSOForParticle(PSOFlags::ForAddBlendParticle());

    CreatePSOForText();

    CreatePSOForOffScreen();
    CreatePSOForDLShadowMap();
    CreatePSOForPLShadowMap();
}

void PSOManager::CreatePSOForModel(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;


    uint64_t type = _flags.GetTypeValue();

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[3] = {};
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

    // pointLight用のサンプラ
    staticSamplers[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 比較用フィルタ
    staticSamplers[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER; // 境界外を無効化
    staticSamplers[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplers[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // 影の外は光が当たる
    staticSamplers[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 深度比較
    staticSamplers[2].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[2].ShaderRegister = 2; // s2
    staticSamplers[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



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


    D3D12_DESCRIPTOR_RANGE PLShadowMapRange[1] = {};
    PLShadowMapRange[0].BaseShaderRegister = 2;  // t2 にバインド
    PLShadowMapRange[0].NumDescriptors = 1;
    PLShadowMapRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    PLShadowMapRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE enviromentMapRange[1] = {};
    enviromentMapRange[0].BaseShaderRegister = 3;  // t3 にバインド
    enviromentMapRange[0].NumDescriptors = 1;
    enviromentMapRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    enviromentMapRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[9] = {};

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

    // PLシャドウマップ
    rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[7].DescriptorTable.pDescriptorRanges = PLShadowMapRange;
    rootParameters[7].DescriptorTable.NumDescriptorRanges = _countof(PLShadowMapRange);

    // 環境マップ用のテクスチャ
    rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[8].DescriptorTable.pDescriptorRanges = enviromentMapRange;
    rootParameters[8].DescriptorTable.NumDescriptorRanges = _countof(enviromentMapRange);



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
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc(_flags);
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

    graphicsPipelineStates_[_flags] = pipelineState;

}


void PSOManager::CreatePSOForSprite(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    uint64_t type = _flags.GetTypeValue();

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

    graphicsPipelineStates_[static_cast<uint64_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForLineDrawer(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    uint64_t type = _flags.GetTypeValue();
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
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc(_flags);
#pragma endregion

#pragma region InputLayout
    /// InputLayoutの設定を行う
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "COLOR";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;



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

    graphicsPipelineStates_[static_cast<uint64_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForParticle(PSOFlags _flags)
{
    HRESULT hr = S_FALSE;

    uint64_t type = _flags.GetTypeValue();

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
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc(_flags);
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

    graphicsPipelineStates_[static_cast<uint64_t>(_flags)] = pipelineState;

}

void PSOManager::CreatePSOForOffScreen()
{
    HRESULT hr = S_FALSE;

    PSOFlags flag = PSOFlags::Type::OffScreen | PSOFlags::CullMode::None | PSOFlags::BlendMode::Normal;

    uint64_t type = flag.GetTypeValue();

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
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
    blendDesc = GetBlendDesc(flag);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);
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
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphicsPipelineStateDesc.BlendState = blendDesc;

    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[type] = pipelineState;

}

void PSOManager::CreatePSOForText()
{
    HRESULT hr = S_FALSE;

    PSOFlags flag = PSOFlags::Type::Text | PSOFlags::CullMode::None | PSOFlags::BlendMode::Normal;

    uint64_t type = flag.GetTypeValue();

#pragma region Sampler
    //Samplerの設定
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0-1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 比較しない
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // あらかじめのMipmapを使う
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // PixelShaderで使う
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

    inputElementDescs[2].SemanticName = "COLOR";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);
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

#pragma region Shader
    /// shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ComplieShader(L"TextRenderer.hlsl", L"vs_6_0",L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = ComplieShader(L"TextRenderer.hlsl", L"ps_6_0",L"PSmain");
    assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc = GetBlendDesc(flag);
#pragma endregion

#pragma region RasterizerState
    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);
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

void PSOManager::CreatePSOForDLShadowMap()
{
    HRESULT hr = S_FALSE;

    PSOFlags flag = PSOFlags::Type::DLShadowMap | PSOFlags::CullMode::Back | PSOFlags::BlendMode::Normal | PSOFlags::DepthMode::Comb_mAll_fLessEqual;

    uint64_t type = flag.GetTypeValue();

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

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    //カメラ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // transform
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 1;

    // LightGroup
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].Descriptor.ShaderRegister = 3;



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
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc(flag);
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
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc = GetRasterizerDesc(flag);

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

    graphicsPipelineStates_[static_cast<uint64_t>(type)] = pipelineState;

}

void PSOManager::CreatePSOForPLShadowMap()
{
    PSOFlags flags = PSOFlags::Type::PLShadowMap | PSOFlags::CullMode::Back | PSOFlags::BlendMode::Normal | PSOFlags::DepthMode::Comb_mAll_fLessEqual;
    HRESULT hr = S_FALSE;

    uint64_t type = flags.GetTypeValue();


#pragma region RootSignature

    /// RootSignatrueを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    //RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[2] = {};


    // transform
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // PointLight
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
    rootParameters[1].Descriptor.ShaderRegister = 1;


    descriptionRootSignature.pStaticSamplers = nullptr;
    descriptionRootSignature.NumStaticSamplers = 0;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);         // 配列の長さ

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
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc(flags);
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
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = PSOManager::GetInstance()->ComplieShader(L"PointLightShadowMap.hlsl", L"vs_6_0", L"VSmain");
    assert(vertexShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob = PSOManager::GetInstance()->ComplieShader(L"PointLightShadowMap.hlsl", L"ps_6_0", L"PSmain");
    assert(pixelShaderBlob != nullptr);
    Microsoft::WRL::ComPtr<IDxcBlob>geometryShaderBlob = PSOManager::GetInstance()->ComplieShader(L"PointLightShadowMap.hlsl", L"gs_6_0", L"GSmain");
#pragma endregion

#pragma region BlendState
    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        blendDesc.RenderTarget[i].BlendEnable = FALSE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
#pragma endregion

#pragma region RasterizerState

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 100000;  // シャドウアクネ防止のための深度バイアス
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 1.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

#pragma endregion

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignatures_[type].Get();                                         // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                        // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };	    // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };       // PixelShader
    graphicsPipelineStateDesc.GS = { geometryShaderBlob->GetBufferPointer(), geometryShaderBlob->GetBufferSize() };       // GeometryShader
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

    graphicsPipelineStateDesc.StreamOutput.RasterizedStream = 0;


    // PSOを生成
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));

    graphicsPipelineStates_[static_cast<uint64_t>(type)] = pipelineState;
}

D3D12_BLEND_DESC PSOManager::GetBlendDesc(PSOFlags _flag)
{
    if(!_flag.IsSingleValueSet(PSOFlags::BlendModeMask))
    {
        assert("BlendModeが複数設定されています" && false);
        return {};
    }


    D3D12_BLEND_DESC blendDesc{};


    PSOFlags::BlendMode mode = _flag.GetBlendMode();

    switch (mode)
    {
    case PSOFlags::BlendMode::Normal:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;
    case PSOFlags::BlendMode::Add:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case PSOFlags::BlendMode::Sub:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;
    case PSOFlags::BlendMode::Multiply:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        break;
    case PSOFlags::BlendMode::Screen:
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
    if (_flag.IsSingleValueSet(PSOFlags::CullModeMask) == false)
    {
        assert("CullModeが複数設定されています" && false);
        return {};
    }

    D3D12_RASTERIZER_DESC rasterizerDesc{};

    PSOFlags::CullMode cullMode = _flag.GetCullMode();

    switch (cullMode)
    {
    case PSOFlags::CullMode::None:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        break;
    case PSOFlags::CullMode::Front:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        break;
    case PSOFlags::CullMode::Back:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        break;
    default:
        break;
    }

    return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC PSOManager::GetDepthStencilDesc(PSOFlags _flag)
{

    /*if (!_flag.IsSingleValueSet(PSOFlags::DepthModeMask))
    {
        assert("DepthModeが複数設定されています" && false);
        return {};
    }*/

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};

    // デフォルト値を設定
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = false;

    // DepthModeを取得
    auto depthMode = _flag.GetDepthMode();

    // Disableの場合
    if (depthMode == PSOFlags::DepthMode::Disable)
    {
        depthStencilDesc.DepthEnable = false;
        return depthStencilDesc;
    }

    // 組み合わせフラグの処理
    if (depthMode == PSOFlags::DepthMode::Comb_mAll_fLessEqual)
    {
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        return depthStencilDesc;
    }
    else if (depthMode == PSOFlags::DepthMode::Comb_mZero_fLessEqual)
    {
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        return depthStencilDesc;
    }
    else if (depthMode == PSOFlags::DepthMode::Comb_mAll_fGreater)
    {
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        return depthStencilDesc;
    }

    // 個別フラグの処理
    if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::Enable)))
    {
        depthStencilDesc.DepthEnable = true;
    }

    if (depthStencilDesc.DepthEnable)
    {
        // マスクの設定
        if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::MaskAll)))
        {
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        }
        else if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::MaskZero)))
        {
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        }

        // 比較関数の設定
        if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::FuncLessEqual)))
        {
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        }
        else if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::FuncAlways)))
        {
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        }
        else if (_flag.HasFlag(PSOFlags(PSOFlags::DepthMode::FuncGreater)))
        {
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        }
    }

    return depthStencilDesc;
}