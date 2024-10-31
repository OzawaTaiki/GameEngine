#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include "ParticleManager.h"
#include <chrono>
#include <imgui.h>

GameScene::~GameScene()
{
    delete color;
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
    ParticleManager::GetInstance()->CreateParticleGroup("sample", "cube/cube.obj",0);

    model_ = Model::CreateFromObj("bunny.gltf");
    trans_.Initialize();
    trans_.UpdateData();
    color = new ObjectColor;
    color->Initialize();
}

void GameScene::Update()
{
    //ImGui::ShowDemoWindow();
    ImGui::Begin("Engine");

    input_->Update();
    //<-----------------------
    camera_->Update();


    ParticleManager::GetInstance()->Update();

    camera_->UpdateMatrix();
    //<-----------------------
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------
    model_->Draw(trans_, camera_.get(), color);

    //<------------------------

    ParticleManager::GetInstance()->Draw(camera_.get());

    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer_->Draw();


}
