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
    Type_AnimationModel		= 1 << 1,
    Type_Sprite				= 1 << 2,
    Type_LineDrawer			= 1 << 3,
    Type_Particle			= 1 << 4,
    Type_OffScreen          = 1 << 5,

    Blend_Normal			= 1 << 11,
    Blend_Add				= 1 << 12,
    Blend_Sub				= 1 << 13,
    Blend_Multiply			= 1 << 14,
    Blend_Screen			= 1 << 15,

    Cull_None				= 1 << 20,
    Cull_Front				= 1 << 21,
    Cull_Back				= 1 << 22,
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
PSOFlags::Type_Model | PSOFlags::Type_AnimationModel |
PSOFlags::Type_Sprite | PSOFlags::Type_LineDrawer | PSOFlags::Type_Particle
| PSOFlags::Type_OffScreen; // 0～4ビット

constexpr PSOFlags BlendMask =
PSOFlags::Blend_Normal | PSOFlags::Blend_Add |
PSOFlags::Blend_Sub | PSOFlags::Blend_Multiply | PSOFlags::Blend_Screen; // 5～9ビット

constexpr PSOFlags CullMask =
PSOFlags::Cull_Back | PSOFlags::Cull_Front | PSOFlags::Cull_None; // 10～12ビット


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

private:

	Microsoft::WRL::ComPtr<IDxcBlob>  ComplieShader(
		//Complierするshaderファイルへのパス
		const std::wstring& _filePath,
		//Compilerに使用するprofile
		const wchar_t* _profile,
		const std::wstring& _dirPath = L"Resources/Shader/");

	void CreatePSOForModel(PSOFlags _flags);
	void CreatePSOForAnimationModel(PSOFlags _flags);
    void CreatePSOForSprite(PSOFlags _flags);
    void CreatePSOForLineDrawer(PSOFlags _flags);
    void CreatePSOForParticle(PSOFlags _flags);
    void CreatePSOForOffScreen();

	D3D12_BLEND_DESC GetBlendDesc(PSOFlags _flag);
    D3D12_RASTERIZER_DESC GetRasterizerDesc(PSOFlags _flag);

    size_t GetType(PSOFlags _flag);
	PSOFlags GetBlendMode(PSOFlags _flag);

	DXCommon* dxCommon_ = nullptr;

	std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> graphicsPipelineStates_;
	std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures_;


    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;





	PSOManager() = default;
	~PSOManager() = default;
	PSOManager(const PSOManager&) = delete;
	PSOManager& operator=(const PSOManager&) = delete;
};
