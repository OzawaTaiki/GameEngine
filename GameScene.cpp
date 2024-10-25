#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
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

    lineDrawer = LineDrawer::GetInstance();
    lineDrawer->SetCameraPtr(camera_.get());

    model = Model::CreateFromObj("cube/cube.obj");
    trans.Initialize();
    color = new ObjectColor;
    color->Initialize();

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


    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::string str_us = std::to_string(duration_us);
    std::string str_ns = std::to_string(duration_ns);
    ImGui::Text("%s us", str_us.c_str());
    ImGui::Text("%s ns", str_ns.c_str());


    trans.TransferData(camera_->GetViewProjection());



    //<-----------------------
    camera_->TransferData();
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------

    model->Draw(trans, camera_.get(),0, color);


    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer->Draw();


}
