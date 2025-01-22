#include "MyGame.h"

#include "SampleScene.h"
#include <Framework/eScene/ParticleTestScene.h>
#include "SceneFactory.h"

void Sample::Initialize()
{
    Framework::Initialize();


    JsonHub::GetInstance()->Initialize("Resources/Data/");


    sceneManager_->SetSceneFactory(new SceneFactory());

    // 最初のシーンで初期化
    sceneManager_->Initialize("Sample");
}

void Sample::Update()
{
    Framework::Update();

    collisionManager_->ResetColliderList();

    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい
    collisionManager_->CheckAllCollision();

    //=============================
}

void Sample::Draw()
{
    Framework::PreDraw();

    // ========== 描画処理 ==========

    sceneManager_->Draw();

    lineDrawer_->Draw();
    //=============================

    Framework::PostDraw();

    // 後にupdateに
    sceneManager_->ChangeScene();

}

void Sample::Finalize()
{
    Framework::Finalize();
}