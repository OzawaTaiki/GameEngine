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

}
void GameScene::Update()
{
    ImGui::Begin("Engine");
    input_->Update();
    //<-----------------------
    camera_->Update();

    lineDrawer->RegisterPoint({ 0,0,15 }, {  3,0,15 });
    lineDrawer->RegisterPoint({ 0,1,15 }, {  1,1,15 });
    lineDrawer->RegisterPoint({ 0,2,15 }, {  3,2,15 });
    lineDrawer->RegisterPoint({ 0,3,15 }, {  3,9,15 });
    lineDrawer->RegisterPoint({ 0,4,15 }, {  3,4,15 });
    lineDrawer->RegisterPoint({ 0,5,15 }, {  3,5,15 });

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
