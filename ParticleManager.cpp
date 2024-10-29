#include "ParticleManager.h"
#include "DXCommon.h"
#include "PSOManager.h"
#include <cassert>

void ParticleManager::Initialize()
{
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("Particle");
    assert(pso.has_value());
    pipelineState_ = pso.value();

   auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Particle");
   assert(rootSignature.has_value());
   rootsignature_ = rootSignature.value();
 //TODO: SRVの設定

}

void ParticleManager::Update()
{
}

void ParticleManager::Draw()
{
}

void ParticleManager::PreDraw()
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetPipelineState(pipelineState_);
    commandList->SetGraphicsRootSignature(rootsignature_);


}
