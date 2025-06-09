#pragma once
#include <Features/Model/Model.h>
#include <Features/Effect/Particle/Particle.h>
#include <Features/Effect/Modifier/ParticleModifier.h>
#include <Features/Effect/Modifier/IPaticleMoifierFactory.h>
#include <Core/BlendMode.h>
#include <System/Time/GameTime.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Camera/Camera/Camera.h>

#include <map>
#include <list>
#include <string>
#include <memory>


#include <d3d12.h>

struct ParticleRenderSettings
{
    BlendMode blendMode = BlendMode::Add;
    bool cullBack = false;

    PSOFlags GetPSOFlags() const
    {
        PSOFlags flags = PSOFlags::BlendMode::Normal;

        switch (blendMode)
        {
        case BlendMode::Normal:
            flags = flags | PSOFlags::BlendMode::Normal;
            break;
        case BlendMode::Add:
            flags = flags | PSOFlags::BlendMode::Add;
            break;
        case BlendMode::Sub:
            flags = flags | PSOFlags::BlendMode::Sub;
            break;
        case BlendMode::Multiply:
            flags = flags | PSOFlags::BlendMode::Multiply;
            break;
        case BlendMode::Screen:
            flags = flags | PSOFlags::BlendMode::Screen;
            break;
        default:
            break;
        }

        if (cullBack)
            flags = flags | PSOFlags::CullMode::Back;
        else
            flags = flags | PSOFlags::CullMode::None;


        return flags;
    }
};

class SRVManager;
class ParticleSystem
{
public:

    static ParticleSystem* GetInstance();

    ParticleSystem() = default;
    ~ParticleSystem();

    void Initialize();
    void Update(float _deltaTime = 1.0f / 60.0f);
    void DrawParticles();

    void SetModifierFactory(IParticleMoifierFactory* _factory);

    void AddParticle(const std::string& _groupName,const std::string& _useModelName, Particle* _particle, ParticleRenderSettings _settings, uint32_t _textureHandle, std::vector<std::string> _modifiers);
    void AddParticles(const std::string& _groupName,const std::string& _useModelName, std::vector<Particle*> _particles, ParticleRenderSettings _settings, uint32_t _textureHandle, std::vector<std::string> _modifiers);


    void ClearParticles();
    void ClearParticles(const std::string& _groupName);

    void SetCamera(Camera* _camera) { camera_ = _camera; }

private:

    // モディファイアをファクトリから生成する
    void CreateModifier(const std::string& _name);

private:

    static uint32_t maxInstancesPerGroup;

    struct ParticleKey
    {
        std::string modelName;
        ParticleRenderSettings settings;

        bool operator<(const ParticleKey& other) const
        {
            // まずモデル名で比較
            if (modelName != other.modelName)
                return modelName < other.modelName;

            // モデル名が同じ場合は設定で比較
            if (settings.blendMode != other.settings.blendMode)
                return settings.blendMode < other.settings.blendMode;

            // 最後にcullBackで比較
            return settings.cullBack < other.settings.cullBack;
        }
    };

    struct ParticleForGPU
    {
        Matrix4x4 worldMatrix;
        Vector4 color;
    };

    struct ParticleGroup
    {
        ParticleKey key;
        Model* model = nullptr;
        std::list <std::unique_ptr<Particle>> particles;
        uint32_t srvIndex = 0;
        uint32_t instanceCount = 0;
        PSOFlags psoIndex = {};
        uint32_t textureHandle = 0;
        std::map<std::string, uint32_t> useModifierName;
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
        ParticleForGPU* mappedInstanceBuffer = nullptr;
    };

    std::map<std::string, ParticleGroup> particles_;

    std::map<PSOFlags, ID3D12PipelineState*> psoMap_;
    ID3D12RootSignature* rootSignature_;

    IParticleMoifierFactory* factory_ = nullptr;

    SRVManager* srvManager_ = nullptr;

    Camera* camera_ = nullptr;

    std::map<std::string, std::unique_ptr<ParticleModifier>> modifierNames_;


};
