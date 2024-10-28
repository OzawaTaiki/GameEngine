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

    lineDrawer = LineDrawer::GetInstance();
    lineDrawer->SetCameraPtr(camera_.get());

    edit_ = std::make_unique<CatmulRomSpline>();
    edit_->Initialize("Resources/Data/Spline");

}

void GameScene::Update()
{
    ImGui::Begin("Engine");

    input_->Update();
    //<-----------------------
    camera_->Update();

    edit_->Update(camera_->GetViewProjection());

    if (edit_->IsMove())
    {
        camera_->matView_ = edit_->GetCamera()->matView_;
        camera_->matProjection_= edit_->GetCamera()->matProjection_;
        camera_->TransferData();
    }
    else
    {
        camera_->UpdateMatrix();
    }


    //<-----------------------
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------

    edit_->Draw(camera_.get());

    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer->Draw();


}
