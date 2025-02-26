#pragma once
#include <Features/Model/Model.h>
#include <Features/Effect/Particle/Particle.h>
#include <Core/BlendMode.h>
#include <System/Time/GameTime.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

#include <unordered_map>
#include <d3d12.h>

class SRVManager;
class ParticleEmitter;
class ParticleManager
{
public:

    static ParticleManager* GetInstance();

    ParticleManager() = default;
    ~ParticleManager();

    void ClearGroup();
    void ClearGroup(const std::string& _groupName);

    void Initialize();
    void Update(const Vector3& _cRotate);
    void Draw(const Camera* _camera);

    void CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath, ParticleEmitter* _emitterPtr, BlendMode _blendMode = BlendMode::Add, uint32_t _textureHandle = UINT32_MAX);
    void CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath, ParticleEmitter* _emitterPtr, PSOFlags _flags, uint32_t _textureHandle = UINT32_MAX);
    void DeleteParticleGroup(const std::string& _groupName);

    void SetGroupModel(const std::string& _groupName, const std::string& _modelPath);
    void SetGroupTexture(const std::string& _groupName, uint32_t _textureHandle);

    void AddParticleToGroup(const std::string& _groupName, const Particle& _particles);
    void AddParticleToGroup(const std::string& _groupName, const std::vector<Particle>& _particles);

    void SetAllGroupTimeChannel(const std::string& _channel);
    void SetGroupTimeChannel(const std::string& _groupName, const std::string& _channel);


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
        BlendMode blendMode;
        PSOFlags psoFlags;
    };

    void PreDraw();

    std::unordered_map <std::string, Group> groups_;

    GameTime* gameTime_ = nullptr;


    std::map<BlendMode,ID3D12PipelineState*> pipelineState_;
    ID3D12RootSignature* rootsignature_;

    SRVManager* srvManager_ = nullptr;
};
