#pragma once


#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <cassert>
#include <unordered_map>
#include <wrl.h>
#include <array>
#include <optional>
#include <stdexcept>

struct PSOFlags
{
    /// enumたちの定義

    enum class Type
    {
        None = 0,
        Model = 1 << 0,
        Sprite = 1 << 1,
        LineDrawer = 1 << 2,
        Particle = 1 << 3,
        OffScreen = 1 << 4,
        DLShadowMap = 1 << 5,
        PLShadowMap = 1 << 6,
        SkyBox = 1 << 7,
        Text = 1 << 8, // テキスト描画用
        Composite = 1 << 9, // レイヤー合成用

    };
    enum class BlendMode
    {
        Normal = 0,
        Add = 1 << 0,
        Sub = 1 << 1,
        Multiply = 1 << 2,
        Screen = 1 << 3,
        PremultipliedAdd = 1 << 4  // レイヤー合成用（アルファ乗算なし）
    };
    enum class CullMode
    {
        None = 0,
        Front = 1,
        Back = 1 << 1
    };
    enum class DepthMode
    {
        Disable = 0,
        Enable = 1 << 0,
        MaskAll = 1 << 1, // すべての深度を書き込む
        MaskZero = 1 << 2, // 深度を書き込まない
        FuncLessEqual = 1 << 3, // 深度比較関数: 手前と等しい場合に描画
        FuncAlways = 1 << 4, // 常に描画する
        FuncGreater = 1 << 5,  // 深度比較関数: 手前より奥のものを描画
        // 通常描画用
        Comb_mAll_fLessEqual = Enable | MaskAll | FuncLessEqual,
        // 深度書き込まない パーティクルなど
        Comb_mZero_fLessEqual = Enable | MaskZero | FuncLessEqual,
        // 書き込まない 奥を描画する 壁の向こうのシルエットとか
        Comb_mAll_fGreater = Enable | MaskAll | FuncGreater,

        Comb_mZero_fAlways = Enable | MaskZero | FuncAlways, // 書き込まない 常に描画する
    };

    static constexpr uint64_t TypeShift = 0;          // Typeのビットシフト
    static constexpr uint64_t BlendModeShift = 12;    // BlendModeのビットシフト
    static constexpr uint64_t CullModeShift = 28;     // CullModeのビットシフト
    static constexpr uint64_t DepthModeShift = 24;    // DepthModeのビットシフト

    // コンストラクタ
    constexpr PSOFlags() = default;
    constexpr PSOFlags(uint64_t val) : value(val) {}
    constexpr PSOFlags(Type type) : value(static_cast<uint64_t>(type)) {}
    constexpr PSOFlags(BlendMode blendMode) : value(static_cast<uint64_t>(blendMode) << BlendModeShift) {}
    constexpr PSOFlags(CullMode cullMode) : value(static_cast<uint64_t>(cullMode) << CullModeShift) {}
    constexpr PSOFlags(DepthMode depthMode) : value(static_cast<uint64_t>(depthMode) << DepthModeShift) {}

    // フラグの値
    uint64_t value = 0;


    // 各フラグのビットマスク
    static constexpr uint64_t TypeMask =        0x0000000000000FFF;
    static constexpr uint64_t BlendModeMask =   0x00000000000FF000;
    static constexpr uint64_t CullModeMask =    0x0000000000F00000;
    static constexpr uint64_t DepthModeMask =   0x0000000FFF000000;

    constexpr operator uint64_t() const { return static_cast<uint64_t>(value); }

    // 比較演算子
    constexpr bool operator==(const PSOFlags& other) const { return value == other.value; }
    constexpr bool operator!=(const PSOFlags& other) const { return value != other.value; }

    constexpr PSOFlags operator|(const PSOFlags& other) const { return PSOFlags(value | other.value); }
    constexpr PSOFlags operator&(const PSOFlags& other) const { return PSOFlags(value & other.value); }

    // 指定されたマスク範囲で単一の値（0を含む）がセットされているか確認
    bool IsSingleValueSet(uint64_t _mask) const
    {
        uint64_t maskedValue = value & _mask;
        // 0は有効な単一値として扱う
        // 0でない場合は、単一ビットかどうかを確認
        return (maskedValue == 0) || ((maskedValue & (maskedValue - 1)) == 0);
    }

    // 0を除外したい場合用）
    bool IsSingleBitSet(uint64_t _mask) const
    {
        uint64_t maskedValue = value & _mask;
        return (maskedValue != 0) && ((maskedValue & (maskedValue - 1)) == 0);
    }

    // Typeがセットされているか確認
    bool HasType() const { return (value & TypeMask) != 0; }
    // 特定のフラグがセットされているか確認
    bool HasFlag(PSOFlags flag) const { return (value & static_cast<uint64_t>(flag)) != 0; }

    Type GetType() const
    {
        if (!HasType())
        {
            //throw std::runtime_error("Typeが設定されていません");
            assert("Typeが設定されていません" && false);
            return Type::None;
        }
        return static_cast<Type>(value & TypeMask);
    }

    uint64_t GetTypeValue() const { return value & TypeMask; }

    // BlendModeを取得
    BlendMode GetBlendMode() const { return static_cast<BlendMode>((value & BlendModeMask) >> BlendModeShift); }
    // CullModeを取得
    CullMode GetCullMode() const { return static_cast<CullMode>((value & CullModeMask) >> CullModeShift); }
    // DepthModeを取得
    DepthMode GetDepthMode() const { return static_cast<DepthMode>((value & DepthModeMask) >> DepthModeShift); }

    // 複数項目の組み合わせを確実に行うヘルパー関数
    static constexpr PSOFlags Combine(Type type, BlendMode blendMode, CullMode cullMode, DepthMode depthMode) {
        return PSOFlags(static_cast<uint64_t>(type) |
            (static_cast<uint64_t>(blendMode) << BlendModeShift) |
            (static_cast<uint64_t>(cullMode)  << CullModeShift) |
            (static_cast<uint64_t>(depthMode) << DepthModeShift));
    }

    static constexpr PSOFlags ForNormalModel() {
        return Combine(Type::Model, BlendMode::Normal, CullMode::Back, DepthMode::Comb_mAll_fLessEqual);
    }
    static constexpr PSOFlags ForAlphaModel() {
        return Combine(Type::Model, BlendMode::Normal, CullMode::Back, DepthMode::Comb_mZero_fLessEqual);
    }
    static constexpr PSOFlags ForSprite() {
        return Combine(Type::Sprite, BlendMode::Normal, CullMode::None, DepthMode::Disable);
    }
    static constexpr PSOFlags ForLineDrawer() {
        return Combine(Type::LineDrawer, BlendMode::Normal, CullMode::None, DepthMode::Comb_mZero_fLessEqual);
    }
    static constexpr PSOFlags ForLineDrawerAlways() {
        return Combine(Type::LineDrawer, BlendMode::Normal, CullMode::None, DepthMode::Comb_mZero_fAlways);
    }
    static constexpr PSOFlags ForAddBlendParticle() {
        return Combine(Type::Particle, BlendMode::Add, CullMode::None, DepthMode::Comb_mZero_fLessEqual);
    }
    static constexpr PSOFlags ForText() {
        return Combine(Type::Text, BlendMode::Normal, CullMode::None, DepthMode::Disable);
    }
};

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

#pragma region psoflagの演算子
// 2項目の組み合わせ
constexpr PSOFlags operator|(PSOFlags::Type lhs, PSOFlags::BlendMode rhs)
{
    return PSOFlags(static_cast<uint64_t>(lhs) | (static_cast<uint64_t>(rhs) << PSOFlags::BlendModeShift));
}
constexpr PSOFlags operator|(PSOFlags::Type lhs, PSOFlags::CullMode rhs)
{
    return PSOFlags(static_cast<uint64_t>(lhs) | (static_cast<uint64_t>(rhs) << PSOFlags::CullModeShift));
}
constexpr PSOFlags operator|(PSOFlags::Type lhs, PSOFlags::DepthMode rhs)
{
    return PSOFlags(static_cast<uint64_t>(lhs) | (static_cast<uint64_t>(rhs) << PSOFlags::DepthModeShift));
}
constexpr PSOFlags operator|(PSOFlags::BlendMode lhs, PSOFlags::CullMode rhs)
{
    return PSOFlags((static_cast<uint64_t>(lhs) << PSOFlags::BlendModeShift) | (static_cast<uint64_t>(rhs) << PSOFlags::CullModeShift));
}
constexpr PSOFlags operator|(PSOFlags::BlendMode lhs, PSOFlags::DepthMode rhs)
{
    return PSOFlags((static_cast<uint64_t>(lhs) << PSOFlags::BlendModeShift) | (static_cast<uint64_t>(rhs) << PSOFlags::DepthModeShift));
}
constexpr PSOFlags operator|(PSOFlags::CullMode lhs, PSOFlags::DepthMode rhs)
{
    return PSOFlags((static_cast<uint64_t>(lhs) << PSOFlags::CullModeShift) | (static_cast<uint64_t>(rhs) << PSOFlags::DepthModeShift));
}

// PSOFlagsとenum classの組み合わせ
constexpr PSOFlags operator|(PSOFlags lhs, PSOFlags::Type rhs)
{
    return lhs | PSOFlags(rhs);
}
constexpr PSOFlags operator|(PSOFlags lhs, PSOFlags::BlendMode rhs)
{
    return lhs | PSOFlags(rhs);
}
constexpr PSOFlags operator|(PSOFlags lhs, PSOFlags::CullMode rhs)
{
    return lhs | PSOFlags(rhs);
}
constexpr PSOFlags operator|(PSOFlags lhs, PSOFlags::DepthMode rhs)
{
    return lhs | PSOFlags(rhs);
}

// 逆順も対応
constexpr PSOFlags operator|(PSOFlags::Type lhs, PSOFlags rhs)
{
    return PSOFlags(lhs) | rhs;
}
constexpr PSOFlags operator|(PSOFlags::BlendMode lhs, PSOFlags rhs)
{
    return PSOFlags(lhs) | rhs;
}
constexpr PSOFlags operator|(PSOFlags::CullMode lhs, PSOFlags rhs)
{
    return PSOFlags(lhs) | rhs;
}
constexpr PSOFlags operator|(PSOFlags::DepthMode lhs, PSOFlags rhs)
{
    return PSOFlags(lhs) | rhs;
}
#pragma endregion