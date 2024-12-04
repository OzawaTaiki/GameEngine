#include "GameScene.h"
#include "Model/ModelManager.h"
#include "Sprite/Sprite.h"
#include "Math/VectorFunction.h"
#include "Math/MatrixFunction.h"
#include "Collision/CollisionManager.h"
#include "ImGuiManager/ImGuiManager.h"
#include <Utility/ConfigManager.h>

std::unique_ptr<BaseScene> GameScene::Create()
{
    return std::make_unique<GameScene>();
}

GameScene::~GameScene()
{
}


void GameScene::Initialize()
{

    input_ = Input::GetInstance();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();
    camera_->translate_ = Vector3{ 0,18,-50 };
    camera_->rotate_ = Vector3{ 0.34f,0,0 };

    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize();

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->SetCameraPtr(camera_.get());

    audio_ = std::make_unique<Audio>();
    audio_->Initialize();

}

void GameScene::Update()
{
#ifdef _DEBUG
    if (input_->IsKeyPressed(DIK_RSHIFT) && Input::GetInstance()->IsKeyTriggered(DIK_RETURN))
    {
        activeDebugCamera_ = !activeDebugCamera_;
    }
#endif // _DEBUG

    input_->Update();
    CollisionManager::GetInstance()->ResetColliderList();


    //<-----------------------
    camera_->Update(0);
    // プレイヤー

    if (activeDebugCamera_)
    {
        debugCamera_->Update();
        camera_->matView_ = debugCamera_->matView_;
        camera_->TransferData();
    }

    else {
        // 追従カメラの更新
        camera_->UpdateMatrix();

    }


    //camera_->UpdateMatrix();
    camera_->TransferData();


    //ParticleManager::GetInstance()->Update(camera_.get());
    CollisionManager::GetInstance()->CheckAllCollision();
    //<-----------------------
}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();
    //<------------------------



    //<------------------------

    ModelManager::GetInstance()->PreDrawForAnimationModel();
    //<------------------------


    ParticleManager::GetInstance()->Draw(camera_.get());
    //<------------------------

    Sprite::PreDraw();
    //<------------------------



    //<------------------------
    lineDrawer_->Draw();


}
