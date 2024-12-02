#include "ParticleTestScene.h"

#include "SceneManager.h"

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
}

void ParticleTestScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void ParticleTestScene::ImGui()
{

}
#endif // _DEBUG