#include <Framework/Framework.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Light/System/LightingSystem.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Json/JsonHub.h>


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

    Time::Update();
    input_->Update();
}

void Framework::PreDraw()
{
    dxCommon_->PreDraw();
    srvManager_->PreDraw();
}

void Framework::PostDraw()
{
    imguiManager_->End();
    imguiManager_->Draw();

    dxCommon_->PostDraw();
}

void Framework::Finalize()
{
    imguiManager_->Finalize();
    delete imguiManager_;

    winApp_->Finalize();
}

