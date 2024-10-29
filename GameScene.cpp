#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include <chrono>
#include <imgui.h>

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
    input_ = Input::GetInstance();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->SetCameraPtr(camera_.get());


    audio_ = std::make_unique<Audio>();
    audio_->Initialize();

    handle_ = audio_->SoundLoadWave("./Resources/sounds/Alarm01.wav");
}

void GameScene::Update()
{
    ImGui::Begin("Engine");

    input_->Update();
    //<-----------------------
    camera_->Update();

    audio_->SoundPlay(handle_, 1.0f, true, false);


    camera_->UpdateMatrix();
    //<-----------------------
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

    lineDrawer_->Draw();


}
