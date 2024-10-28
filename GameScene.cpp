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

    ImGui::DragFloat3("trans", &trans.transform_.x, 0.01f);
    ImGui::DragFloat3("rotate", &trans.rotate_.x, 0.01f);
    ImGui::DragFloat3("anchorPos", &anchorPos.x, 0.01f);
    ImGui::DragFloat3("anchorRot", &anchorRot.x, 0.01f);

    input_->Update();
    //<-----------------------
    camera_->Update();
    auto start = std::chrono::high_resolution_clock::now();

    trans.matWorld_ = MakeAffineMatrix(trans.scale_, trans.rotate_, trans.transform_);

    Matrix4x4 Mpos = MakeTranslateMatrix(anchorPos - trans.transform_);
    Matrix4x4 MRot = MakeRotateMatrix(anchorRot);


    trans.matWorld_ *= Mpos;
    trans.matWorld_ *= MRot;
    trans.matWorld_ *= Inverse(Mpos);

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
