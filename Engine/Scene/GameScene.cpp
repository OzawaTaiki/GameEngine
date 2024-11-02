#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include "ParticleManager.h"
#include "TextureManager.h"
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

    model_ = Model::CreateFromObj("bunny.gltf");
    trans_.Initialize();
    trans_.UpdateData();
    color = new ObjectColor;
    color->Initialize();

    emit_ = new ParticleEmitter;
    emit_->Setting({ 0,0,0 }, { 0,0,0 }, 1, 1, 10, true);
    emit_->SetShape_Box({ 2, 2, 2 });

   uint32_t handle= TextureManager::GetInstance()->Load("circle.png");
   ParticleManager::GetInstance()->CreateParticleGroup("sample", "plane/plane.obj", emit_, handle);
}

void GameScene::Update()
{
    //ImGui::ShowDemoWindow();
    ImGui::Begin("Engine");

    input_->Update();
    //<-----------------------
    camera_->Update();

    emit_->Update();
    ParticleManager::GetInstance()->Update();

    camera_->UpdateMatrix();
    //<-----------------------
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------
    //model_->Draw(trans_, camera_.get(), color);

    //<------------------------

    ParticleManager::GetInstance()->Draw(camera_.get());

    Sprite::PreDraw();
    //<------------------------



    //<------------------------
    emit_->Draw();
    lineDrawer_->Draw();


}
