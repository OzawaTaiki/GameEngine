#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>
#include <unordered_map>

class ShaderCompiler {
public:
    static ShaderCompiler* GetInstance();

    void Initialize();

    // シェーダコンパイル（キャッシュ対応）
    Microsoft::WRL::ComPtr<IDxcBlob> Compile(
        const std::wstring& _filePath,
        const wchar_t* _profile,
        const std::wstring& _entryFuncName = L"main",
        const std::wstring& _dirPath = L"Resources/Shader/"
    );

    // シェーダを名前で登録
    void Register(
        const std::string& _name,
        const std::wstring& _filePath,
        const wchar_t* _profile,
        const std::wstring& _entryFuncName = L"main",
        const std::wstring& _dirPath = L"Resources/Shader/"
    );

    // 名前でシェーダ取得（未コンパイルなら自動コンパイル）
    Microsoft::WRL::ComPtr<IDxcBlob> Get(const std::string& _name);

    // よく使うシェーダを一括登録
    void RegisterCommonShaders();

    // キャッシュクリア
    void ClearCache();

private:
    ShaderCompiler() = default;
    ~ShaderCompiler() = default;
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    // キャッシュ: フルパス → コンパイル済みBlob
    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<IDxcBlob>> cache_;

    // Dictionary: 名前 → シェーダ情報
    struct ShaderInfo {
        std::wstring filePath;
        std::wstring profile;
        std::wstring entryFuncName;
        std::wstring dirPath;
    };
    std::unordered_map<std::string, ShaderInfo> dictionary_;

    // キャッシュキー生成
    std::wstring MakeCacheKey(
        const std::wstring& _filePath,
        const wchar_t* _profile,
        const std::wstring& _entryFuncName,
        const std::wstring& _dirPath
    );
};
