#pragma once

#include "PSOFlags.h"

#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <cassert>
#include <unordered_map>
#include <wrl.h>
#include <array>
#include <optional>
#include <stdexcept>

class DXCommon;
class PSOManager
{
public:
    static PSOManager* GetInstance();

    void Initialize();

    std::optional<ID3D12PipelineState*> GetPipeLineStateObject(PSOFlags _flag);
    std::optional<ID3D12RootSignature*> GetRootSignature(PSOFlags _flag);

    void SetPipeLineStateObject(PSOFlags _flag);
    void SetRootSignature(PSOFlags _flag);

    std::optional<ID3D12PipelineState*> GetPSO(const std::string& name);

    void SetPSOForPostEffect(const std::string& _name);

    void SetRegisterPSO(const std::string& _name);
    void SetRegisterRootSignature(const std::string& _name);

    Microsoft::WRL::ComPtr<IDxcBlob>  ComplieShader(
        //Complierするshaderファイルへのパス
        const std::wstring& _filePath,
        //Compilerに使用するprofile
        const wchar_t* _profile,
        const std::wstring& _entryFuncName = L"main",
        const std::wstring& _dirPath = L"Resources/Shader/"
    );

    void CreatePSOForPostEffect(const std::string& _name,
        const std::wstring& _psFileName,
        const std::wstring& _entryFuncName = L"main",
        const std::wstring& _dirPath = L"Resources/Shader/");

    void CreatePSOForComposite(PSOFlags::BlendMode _blendMode);

    void RegisterPSO(const std::string& _name, ID3D12PipelineState* _pso);
    void RegisterRootSignature(const std::string& _name, ID3D12RootSignature* _rs);

    void CreatePSOForSkyBox();

    static D3D12_BLEND_DESC GetBlendDesc(PSOFlags _flag);
    static D3D12_RASTERIZER_DESC GetRasterizerDesc(PSOFlags _flag);
    static D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(PSOFlags _flag);

private:
    void CreateDefaultPSOs();

    void CreatePSOForModel(PSOFlags _flags);
    void CreatePSOForSprite(PSOFlags _flags);
    void CreatePSOForLineDrawer(PSOFlags _flags);
    void CreatePSOForParticle(PSOFlags _flags);
    void CreatePSOForOffScreen();
    void CreatePSOForText();

    void CreatePSOForDLShadowMap();
    void CreatePSOForPLShadowMap();


    DXCommon* dxCommon_ = nullptr;

    std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> graphicsPipelineStates_;
    std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> postEffectPipelineStates_;

    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> registerPSO_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> regiterRootSignature_;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    PSOManager() = default;
    ~PSOManager() = default;
    PSOManager(const PSOManager&) = delete;
    PSOManager& operator=(const PSOManager&) = delete;
};
