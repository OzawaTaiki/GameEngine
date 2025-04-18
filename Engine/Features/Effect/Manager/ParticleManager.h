#pragma once
#include <Features/Model/Model.h>
#include <Features/Effect/Particle/Particle.h>
#include <Core/BlendMode.h>
#include <System/Time/GameTime.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Camera/Camera/Camera.h>

#include <map>
#include <list>


#include <d3d12.h>

struct ParticleRenderSettings
{
    BlendMode blendMode = BlendMode::Add;
    bool cullBack = false;

    PSOFlags GetPSOFlags() const
    {
        PSOFlags flags = PSOFlags::None;

        switch (blendMode)
        {
        case BlendMode::Normal:
            flags |= PSOFlags::Blend_Normal;
            break;
        case BlendMode::Add:
            flags |= PSOFlags::Blend_Add;
            break;
        case BlendMode::Sub:
            flags |= PSOFlags::Blend_Sub;
            break;
        case BlendMode::Multiply:
            flags |= PSOFlags::Blend_Multiply;
            break;
        case BlendMode::Screen:
            flags |= PSOFlags::Blend_Screen;
            break;
        default:
            break;
        }

        if (cullBack)
            flags |= PSOFlags::Cull_Back;
        else
            flags |= PSOFlags::Cull_None;


        return flags;
    }
};

class SRVManager;
class ParticleManager
{
public:

    static ParticleManager* GetInstance();

    ParticleManager() = default;
    ~ParticleManager() = default;

    void Initialize();
    void Update(float _deltaTime = 1.0f / 60.0f);
    void DrawParticles();

    void AddParticle(const std::string& _useModelName, Particle* _particle,ParticleRenderSettings _settings, uint32_t _textureHandle);
    void AddParticles(const std::string& _useModelName, std::vector<Particle*> _particles,ParticleRenderSettings _settings, uint32_t _textureHandle);


    void ClearParticles();
    void ClearParticles(const std::string& _useModelName);

    void SetCamera(Camera* _camera) { camera_ = _camera; }

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
        Model* model = nullptr;
        std::list<Particle*> particles;
        uint32_t srvIndex = 0;
        uint32_t instanceCount = 0;
        PSOFlags psoIndex = {};
        uint32_t textureHandle = 0;
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
        ParticleForGPU* mappedInstanceBuffer = nullptr;
    };

    std::map<ParticleKey, ParticleGroup> particles_;

    std::map<PSOFlags, ID3D12PipelineState*> psoMap_;
    ID3D12RootSignature* rootSignature_;

    SRVManager* srvManager_ = nullptr;

    Camera* camera_ = nullptr;



};
