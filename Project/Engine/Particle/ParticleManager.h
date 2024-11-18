#pragma once
#include "Model.h"
#include "Particle.h"

#include <d3d12.h>
class SRVManager;
class ParticleEmitter;
class ParticleManager
{
public:

    static ParticleManager* GetInstance();

    ParticleManager() = default;
    ~ParticleManager();

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

    void CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath, ParticleEmitter* _emitterPtr, uint32_t _textureHandle = UINT32_MAX);

    void AddParticleToGroup(const std::string& _groupName, const std::vector<Particle>& _particles);
private:

    static const uint32_t kGroupMaxInstance;

    struct constantBufferData
    {
        Matrix4x4 matWorld;
        Vector4 color;
    };
    struct Group
    {
        Model* model;
        std::list<Particle>  particles;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        constantBufferData* constMap;
        uint32_t textureHandle;
        uint32_t srvIndex;
        uint32_t instanceNum;
        ParticleEmitter* emitterPtr;
    };

    void PreDraw();

    std::unordered_map <std::string, Group> groups_;

    ID3D12PipelineState* pipelineState_;
    ID3D12RootSignature* rootsignature_;

    SRVManager* srvManager_ = nullptr;
};