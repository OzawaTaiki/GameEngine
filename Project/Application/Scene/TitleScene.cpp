#include "TitleScene.h"

#include "eScene/SceneManager.h"
#include "Input/Input.h"
#include "Model/ModelManager.h"
#include "TextureManager/TextureManager.h"
#include "Utility/ConfigManager.h"

std::unique_ptr<BaseScene>TitleScene::Create()
{
    return std::make_unique<TitleScene>();
}

TitleScene::~TitleScene()
{
}

void TitleScene::Initialize()
{
}

void TitleScene::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void TitleScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void TitleScene::ImGui()
{
}
#endif // _DEBUG