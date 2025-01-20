#include "MyGame.h"

#include <Framework/eScene/SampleScene.h>
#include <Framework/eScene/ParticleTestScene.h>

void MyGame::Initialize()
{
    Framework::Initialize();

    sceneManager_->RegisterScene("Sample", SampleScene::Create);
    sceneManager_->RegisterScene("ParticleTest", ParticleTestScene::Create);
    sceneManager_->Initialize("Sample");

}

void MyGame::Update()
{
    Framework::Update();
}

void MyGame::Draw()
{
    Framework::PreDraw();

    // ========== 描画処理 ==========

    sceneManager_->Draw();

    //=============================

    Framework::PostDraw();
}

void MyGame::Finalize()
{
    Framework::Finalize();
}