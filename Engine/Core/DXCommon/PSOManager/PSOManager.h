#pragma once

#include <Core/BlendMode.h>

#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include <array>
#include <optional>

#pragma region PSOFlags
enum class PSOFlags
{
    None					= 0,

    Type_Model				= 1,
    Type_Sprite				= 1 << 1,
    Type_LineDrawer			= 1 << 2,
    Type_Particle			= 1 << 3,
    Type_OffScreen          = 1 << 4,
    Type_DLShadowMap        = 1 << 5,
    Type_PLShadowMap        = 1 << 6,
    Type_SkyBox             = 1 << 7,

    Blend_Normal			= 1 << 11,
    Blend_Add				= 1 << 12,
    Blend_Sub				= 1 << 13,
    Blend_Multiply			= 1 << 14,
    Blend_Screen			= 1 << 15,

    Cull_None				= 1 << 20,
    Cull_Front				= 1 << 21,
    Cull_Back				= 1 << 22,

    Depth_Enable            = 1 << 23,
    Depth_Disable           = 1 << 24,
    Depth_Mask_All          = 1 << 25,
    Depth_Mask_Zero         = 1 << 26,
    Depth_Func_LessEqual    = 1 << 27,
    Depth_Func_Always       = 1 << 28,
    // TODO 項目ごとに分けるべき

    Depth_mAll_fLEqual = Depth_Enable | Depth_Mask_All | Depth_Func_LessEqual,
    Depth_mZero_fLEqual = Depth_Enable | Depth_Mask_Zero | Depth_Func_LessEqual
};

PSOFlags SetBlendMode(PSOFlags _flag, PSOFlags _mode);

PSOFlags GetBlendMode(BlendMode _mode);

BlendMode GetBlendMode(PSOFlags _flag);

/// <summary>
/// PSOFlagsが有効かどうかを判定する
/// </summary>
/// <param name="_flags"></param>
/// <returns>有効なら true</returns>
bool IsValidPSOFlags(PSOFlags _flags);

/// <summary>
/// PSOFlagsの中に一つだけビットが立っているかどうかを判定する
/// </summary>
/// <param name="_flags"></param>
/// <param name="_mask"></param>
/// <returns>複数設定されていたら false</returns>
bool IsSingleBitSetInMask(PSOFlags _flags, PSOFlags _mask);

/// <summary>
/// PSOFlagsの中に指定したフラグが含まれているかどうかを判定する
/// </summary>
/// <param name="_flag"></param>
/// <param name="_checkFlag"></param>
/// <returns></returns>
bool HasFlag(PSOFlags _flag, PSOFlags _checkFlag);

#pragma region PSOFlagsの演算子オーバーロード
constexpr PSOFlags operator|(PSOFlags a, PSOFlags b)
{
    return static_cast<PSOFlags>(static_cast<int>(a) | static_cast<int>(b));
}
constexpr PSOFlags operator&(PSOFlags a, PSOFlags b)
{
    return static_cast<PSOFlags>(static_cast<int>(a) & static_cast<int>(b));
}
constexpr PSOFlags operator~(PSOFlags a)
{
    return static_cast<PSOFlags>(~static_cast<int>(a));
}
constexpr PSOFlags& operator|=(PSOFlags& a, PSOFlags b)
{
    return a = a | b;
}
constexpr PSOFlags& operator&=(PSOFlags& a, PSOFlags b)
{
    return a = a & b;
}
#pragma endregion

#pragma region PSOFlagsのマスク
constexpr PSOFlags TypeMask =
PSOFlags::Type_Model |
PSOFlags::Type_Sprite | PSOFlags::Type_LineDrawer | PSOFlags::Type_Particle
| PSOFlags::Type_OffScreen | PSOFlags::Type_DLShadowMap | PSOFlags::Type_PLShadowMap
| PSOFlags::Type_SkyBox
;
constexpr PSOFlags BlendMask =
PSOFlags::Blend_Normal | PSOFlags::Blend_Add |
PSOFlags::Blend_Sub | PSOFlags::Blend_Multiply | PSOFlags::Blend_Screen;

constexpr PSOFlags CullMask =
PSOFlags::Cull_Back | PSOFlags::Cull_Front | PSOFlags::Cull_None;

constexpr PSOFlags DepthMask =
PSOFlags::Depth_Enable | PSOFlags::Depth_Disable | PSOFlags::Depth_Mask_All
| PSOFlags::Depth_Mask_Zero | PSOFlags::Depth_Func_LessEqual
| PSOFlags::Depth_Func_Always;



#pragma endregion
#pragma endregion

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

    void RegisterPSO(const std::string& _name, ID3D12PipelineState* _pso);
    void RegisterRootSignature(const std::string& _name, ID3D12RootSignature* _rs);

    void CreatePSOForSkyBox();

private:

    void CreatePSOForModel(PSOFlags _flags);
    void CreatePSOForSprite(PSOFlags _flags);
    void CreatePSOForLineDrawer(PSOFlags _flags);
    void CreatePSOForParticle(PSOFlags _flags);
    void CreatePSOForOffScreen();

    void CreatePSOForDLShadowMap();
    void CreatePSOForPLShadowMap();



	D3D12_BLEND_DESC GetBlendDesc(PSOFlags _flag);
    D3D12_RASTERIZER_DESC GetRasterizerDesc(PSOFlags _flag);
    D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(PSOFlags _flag);

    size_t GetType(PSOFlags _flag);
	PSOFlags GetBlendMode(PSOFlags _flag);

	DXCommon* dxCommon_ = nullptr;

	std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> graphicsPipelineStates_;
	std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures_;
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
