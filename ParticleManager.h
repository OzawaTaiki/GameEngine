#pragma once

#include <d3d12.h>

class ParticleManager
{
public:

    ParticleManager() = default;
    ~ParticleManager() = default;

    void Initialize();
    void Update();
    void Draw();

private:

    void PreDraw();

    ID3D12PipelineState* pipelineState_;
    ID3D12RootSignature* rootsignature_;


};