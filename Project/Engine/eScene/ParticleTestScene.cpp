#include "ParticleTestScene.h"

#include "SceneManager.h"
#include "Utility/ConfigManager.h"
#include "Particle/ParticleManager.h"

std::unique_ptr<BaseScene>ParticleTestScene::Create()
{
    return std::make_unique<ParticleTestScene>();
}

ParticleTestScene::~ParticleTestScene()
{
}

void ParticleTestScene::Initialize()
{
}

void ParticleTestScene::Update()
{
    for (auto& e : emitters_)
    {
        e->Update();
    }

    for (auto& e : effects_)
    {
        e.Update();
    }
}

void ParticleTestScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void ParticleTestScene::ImGui()
{
    ImGui::Begin("ParticleTestScene");

    // エミッターの追加
    if (ImGui::Button("Create New Emitter"))
    {
        emitters_.push_back(std::make_unique<ParticleEmitter>());
    }

    if (!emitters_.empty())
    {

    }

    ImGui::End();
}
#endif // _DEBUG