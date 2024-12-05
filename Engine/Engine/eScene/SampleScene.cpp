#include "SampleScene.h"

#include "SceneManager.h"

std::unique_ptr<BaseScene>SampleScene::Create()
{
    return std::make_unique<SampleScene>();
}

SampleScene::~SampleScene()
{
}

void SampleScene::Initialize()
{
}

void SampleScene::Update()
{
}

void SampleScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG