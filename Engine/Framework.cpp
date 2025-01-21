#include "Framework.h"

#include <Core/DirectX/PSOManager.h>
#include <Rendering/Light/LightingSystem.h>
#include <ResourceManagement/TextureManager/TextureManager.h>
#include <Rendering/Sprite/Sprite.h>
#include <Rendering/Model/ModelManager.h>
#include <Systems/JsonBinder/JsonHub.h>


void Framework::Run()
{
    Initialize();

    while (!IsEndRequested())
    {
        Update();
        Draw();
    }

    Finalize();
}

void Framework::Initialize()
{
    winApp_ = WinApp::GetInstance();
    winApp_->Initilize();

    dxCommon_ = DXCommon::GetInstance();
    dxCommon_->Initialize(winApp_, WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    srvManager_ = SRVManager::GetInstance();
    srvManager_->Initialize();

    PSOManager::GetInstance()->Initialize();

    LightingSystem::GetInstance()->Initialize();

    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize();

    particleManager_ = ParticleManager::GetInstance();
    particleManager_->Initialize();

    TextureManager* instance = TextureManager::GetInstance();
    instance->Initialize();

    Sprite::StaticInitialize(WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    ModelManager::GetInstance()->Initialize();

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();

    input_ = Input::GetInstance();
    input_->Initilize(winApp_);

    JsonHub::GetInstance()->Initialize("Resources/Data/");

    Time::Initialize();

    sceneManager_ = SceneManager::GetInstance();

    collisionManager_ = CollisionManager::GetInstance();
}

void Framework::Update()
{
    if (winApp_->ProcessMessage())
    {
        endRequest_ = true;
    }

    // フレーム始め
    imguiManager_->Begin();

    collisionManager_->ResetColliderList();

    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい
    collisionManager_->CheckAllCollision();

    //=============================
}

void Framework::PreDraw()
{
    dxCommon_->PreDraw();
    srvManager_->PreDraw();
}

void Framework::PostDraw()
{
    lineDrawer_->Draw();

    imguiManager_->End();
    imguiManager_->Draw();

    dxCommon_->PostDraw();
    sceneManager_->ChangeScene();
}

void Framework::Finalize()
{
    imguiManager_->Finalize();
    delete imguiManager_;

    winApp_->Finalize();
}

