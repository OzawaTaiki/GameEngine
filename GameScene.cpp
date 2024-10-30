#include "GameScene.h"
#include "ModelManager.h"
#include "Sprite.h"
#include <imgui.h>

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
    input_ = Input::GetInstance();

    DebugCamera_ = std::make_unique<DebugCamera>();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    lineDrawer = LineDrawer::GetInstance();
    lineDrawer->SetCameraPtr(camera_.get());

    edit_ = std::make_unique<CatmulRomSpline>();
    edit_->Initialize("Resources/Data/Spline");

    tile_ = Model::CreateFromObj("tile/tile.gltf");
    tile_->SetUVScale({ 100,100 });
    trans_.Initialize();
    trans_.UpdateData();

    color_ = std::make_unique<ObjectColor>();
    color_->Initialize();
}

void GameScene::Update()
{
    ImGui::Begin("Engine");
    input_->Update();
    if (input_->IsKeyPressed(DIK_RSHIFT) && input_->IsKeyTriggered(DIK_RETURN))
        useDebugCamera_ = !useDebugCamera_;

    DebugCamera_->Update();
    //<-----------------------
    camera_->Update();

    edit_->Update(camera_->GetViewProjection());


    if (edit_->IsMove() && !useDebugCamera_)
    {
        camera_->matView_ = edit_->GetCamera()->matView_;
        camera_->matProjection_= edit_->GetCamera()->matProjection_;
        camera_->TransferData();
    }
    else if (useDebugCamera_)
    {
        camera_->matView_ = DebugCamera_->matView_;
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
    tile_->Draw(trans_, camera_.get(), color_.get());

    edit_->Draw(camera_.get());

    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer->Draw();


}
