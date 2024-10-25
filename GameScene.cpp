#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
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

    model = Model::CreateFromObj("bunny.gltf");
    trans.Initialize();
    color = new ObjectColor;
    color->Initialize();

    edit_ = std::make_unique<CatmulRomSpline>();
    edit_->Initialize("Resources/Data/Spline");
    edit_->SetCameraPtr(camera_.get());

}

void GameScene::Update()
{
    ImGui::Begin("Engine");
    input_->Update();
    //<-----------------------
    camera_->Update();

    edit_->Update();


    trans.UpdateData();
    //<-----------------------
    camera_->TransferData();
    ImGui::End();
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDraw();
    //<------------------------

    //model->Draw(trans, camera_.get(),0, color);
    edit_->Draw();

    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer->Draw();


}
