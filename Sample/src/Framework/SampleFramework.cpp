#include "SampleFramework.h"

#include <src/Scene/SampleScene.h>
#include <src/Factory/SceneFactory.h>
#include <Features/Scene/ParticleTestScene.h>

void SampleFramework::Initialize()
{
    Framework::Initialize();


    JsonHub::GetInstance()->Initialize("Resources/Data/");


    sceneManager_->SetSceneFactory(new SceneFactory());

    // 最初のシーンで初期化
    sceneManager_->Initialize("Feature");
}

void SampleFramework::Update()
{
    Framework::Update();

    collisionManager_->ResetColliderList();

    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい
    collisionManager_->CheckAllCollision();

    //=============================
}

void SampleFramework::Draw()
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

void SampleFramework::Finalize()
{
    Framework::Finalize();
}
