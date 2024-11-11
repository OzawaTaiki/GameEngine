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

}
void GameScene::Initialize()
{
    input_ = Input::GetInstance();

    DebugCamera_ = std::make_unique<DebugCamera>();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    lineDrawer = LineDrawer::GetInstance();
    lineDrawer->SetCameraPtr(camera_.get());

    tile_ = Model::CreateFromObj("tile/tile.gltf");
    tile_->SetUVScale({ 100,100 });
    trans_.Initialize();
    trans_.UpdateData();


    edit_ = std::make_unique<CatmulRomSpline>();
    edit_->Initialize("Resources/Data/Spline");

    player_ = std::make_unique<Player>();
    player_->Initialize();

    edit_->SetMoveObjTrans(player_->GetWorldTransform());
    camera_->SetParent(player_->GetWorldTransform());
    camera_->translate_ = { 0,1.25f,-0.65f };
}

void GameScene::Update()
{
    ImGui::Begin("Engine");
    input_->Update();
    if (input_->IsKeyPressed(DIK_RSHIFT) && input_->IsKeyTriggered(DIK_RETURN))
        useDebugCamera_ = !useDebugCamera_;

    //<-----------------------
    camera_->Update();

    edit_->Update(camera_->GetViewProjection());

    player_->Update(camera_->GetViewProjection());

    if (useDebugCamera_)
    {
        DebugCamera_->Update();
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
    tile_->Draw(trans_, camera_.get());
    //<------------------------

    player_->Draw(camera_.get());

    edit_->Draw(camera_.get());

    //<------------------------



    Sprite::PreDraw();
    //<------------------------



    //<------------------------

    lineDrawer->Draw();


}