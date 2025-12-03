#include "ShaderCompiler.h"
#include <Debug/Debug.h>
#include <Utility/ConvertString/ConvertString.h>
#include <cassert>
#include <format>


ShaderCompiler* ShaderCompiler::GetInstance() {
    static ShaderCompiler instance;
    return &instance;
}

void ShaderCompiler::Initialize() {
    HRESULT hr = S_FALSE;

    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));

    cache_.clear();
    dictionary_.clear();
    RegisterCommonShaders();
    Debug::Log("ShaderCompiler initialized\n");
}

std::wstring ShaderCompiler::MakeCacheKey(
    const std::wstring& _filePath,
    const wchar_t* _profile,
    const std::wstring& _entryFuncName,
    const std::wstring& _dirPath) {

    return _dirPath + _filePath + L"|" + _profile + L"|" + _entryFuncName;
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(
    const std::wstring& _filePath,
    const wchar_t* _profile,
    const std::wstring& _entryFuncName,
    const std::wstring& _dirPath) {

    // キャッシュチェック
    std::wstring cacheKey = MakeCacheKey(_filePath, _profile, _entryFuncName, _dirPath);
    auto it = cache_.find(cacheKey);
    if (it != cache_.end()) {
        Debug::Log("Shader cache hit: " + ConvertString(cacheKey) + "\n");
        return it->second;
    }

    // キャッシュミス → コンパイル
    std::wstring fullpath = _dirPath + _filePath;

    Debug::Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n",
                                          fullpath, _profile)));

    Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils_->LoadFile(fullpath.c_str(), nullptr, &shaderSource);
    assert(SUCCEEDED(hr));

    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;

    LPCWSTR arguments[] = {
        _filePath.c_str(),
        L"-E", _entryFuncName.c_str(),
        L"-T", _profile,
        L"-Zi", L"-Qembed_debug",
        L"-Od",
        L"-Zpr",
    };

    Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler_->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler_.Get(),
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        Debug::Log(shaderError->GetStringPointer());
        assert(false);
    }

    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));

    // キャッシュに保存
    cache_[cacheKey] = shaderBlob;

    Debug::Log("Shader compiled and cached: " + ConvertString(cacheKey) + "\n");

    return shaderBlob;
}

void ShaderCompiler::Register(
    const std::string& _name,
    const std::wstring& _filePath,
    const wchar_t* _profile,
    const std::wstring& _entryFuncName,
    const std::wstring& _dirPath) {

    ShaderInfo info;
    info.filePath = _filePath;
    info.profile = _profile;
    info.entryFuncName = _entryFuncName;
    info.dirPath = _dirPath;

    dictionary_[_name] = info;

    Debug::Log("Shader registered: " + _name + "\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Get(const std::string& _name) {
    auto it = dictionary_.find(_name);
    if (it == dictionary_.end()) {
        Debug::Log("Shader not found in dictionary: " + _name + "\n");
        assert(false && "Shader not found in dictionary");
        return nullptr;
    }

    const auto& info = it->second;
    return Compile(info.filePath, info.profile.c_str(),
                   info.entryFuncName, info.dirPath);
}

void ShaderCompiler::RegisterCommonShaders() {
    // モデル用
    Register("Model_VS", L"Object3d.VS.hlsl", L"vs_6_0");
    Register("Model_PS", L"Object3d.PS.hlsl", L"ps_6_0");

    // スプライト用
    Register("Sprite_VS", L"Sprite.VS.hlsl", L"vs_6_0");
    Register("Sprite_PS", L"Sprite.PS.hlsl", L"ps_6_0");

    // ポストエフェクト用
    Register("FullScreen_VS", L"FullScreen.VS.hlsl", L"vs_6_0");
    Register("Vignette_PS", L"Vignetting.hlsl", L"ps_6_0");
    Register("GrayScale_PS", L"GrayScale.hlsl", L"ps_6_0");

    // パーティクル用
    Register("Particle_VS", L"Particle.VS.hlsl", L"vs_6_0");
    Register("Particle_PS", L"Particle.PS.hlsl", L"ps_6_0");

    // ライン描画用
    Register("Line_VS", L"Line.VS.hlsl", L"vs_6_0");
    Register("Line_PS", L"Line.PS.hlsl", L"ps_6_0");

    // SkyBox用
    Register("SkyBox_VS", L"SkyBox.hlsl", L"vs_6_0", L"VSMain");
    Register("SkyBox_PS", L"SkyBox.hlsl", L"ps_6_0", L"PSMain");

    Debug::Log("Common shaders registered\n");
}

void ShaderCompiler::ClearCache() {
    cache_.clear();
    Debug::Log("Shader cache cleared\n");
}
