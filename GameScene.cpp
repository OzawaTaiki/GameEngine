#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include <imgui.h>
void GameScene::Initialize()
{
    input_ = Input::GetInstance();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();
}

void GameScene::Update()
{
    ImGui::Begin("Engine");
    input_->Update();
    //<-----------------------





    //<-----------------------
    camera_->TransferData();
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------



    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------


}
