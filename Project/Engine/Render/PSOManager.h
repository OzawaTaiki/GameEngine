#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include <array>
#include <optional>

class DXCommon;
class PSOManager
{
public:

	enum class BlendMode
	{
		Normal,
		Add,
		Sub,
		Multiply,
		Screen,

		ModeNum
	};

	static PSOManager* GetInstance();

    void Initialize();

	std::optional<ID3D12PipelineState*> GetPipeLineStateObject(const std::string& _key, BlendMode _mode = BlendMode::Normal);
	std::optional<ID3D12RootSignature*> GetRootSignature(const std::string& _key);

private:


	Microsoft::WRL::ComPtr<IDxcBlob>  ComplieShader(
		//Complierするshaderファイルへのパス
		const std::wstring& _filePath,
		//Compilerに使用するprofile
		const wchar_t* _profile,
		const std::wstring& _dirPath = L"Resources/Shader/");

	void CreatePSOForModel();
	void CreatePSOForSprite();
	void CreatePSOForLineDrawer();
	void CreatePSOForParticle();

	D3D12_BLEND_DESC GetBlendDesc(BlendMode _mode);

	DXCommon* dxCommon_ = nullptr;

	std::unordered_map<std::string, std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::ModeNum)>> graphicsPipelineStates_;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures_;


    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;





	PSOManager() = default;
	~PSOManager() = default;
	PSOManager(const PSOManager&) = delete;
	PSOManager& operator=(const PSOManager&) = delete;
};