#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include <chrono>
#include <imgui.h>

std::unique_ptr<BaseScene> GameScene::Create()
{
    return std::make_unique<GameScene>();
}

GameScene::~GameScene()
{
    delete color_;
    delete model_;
    delete humanModel_;
    delete emit_;
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

    model_ = new ObjectModel;
    model_->Initialize("bunny.gltf");
    humanModel_ = new AnimationModel;
    humanModel_->Initialize("human/walk.gltf");

}

void GameScene::Update()
{

    //ImGui::ShowDemoWindow();
    ImGui::Begin("Engine");

    input_->Update();
    //<-----------------------
    camera_->Update();


    model_->Update();
    humanModel_->Update();

    camera_->TransferData();
    //<-----------------------
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();
    //<------------------------
    model_->Draw(camera_.get(), { 1,1,1,1 });

    //<------------------------

    ModelManager::GetInstance()->PreDrawForAnimationModel();
    //<------------------------
    humanModel_->Draw(camera_.get(), { 1,1,1,1 });

    //<------------------------

    Sprite::PreDraw();
    //<------------------------



    //<------------------------
    lineDrawer_->Draw();


}
