#include <Framework/Framework.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Light/System/LightingSystem.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Event/EventManager.h>
#include <System/Audio/AudioSystem.h>

#include <Features/Json/JsonHub.h>

#include <Debug/ImGuiDebugManager.h>



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



    rtvManager_ = RTVManager::GetInstance();
    rtvManager_->Initialize(dxCommon_->GetBackBufferSize(), WinApp::kWindowWidth_, WinApp::kWindowHeight_);


    PSOManager::GetInstance()->Initialize();

    LightingSystem::GetInstance()->Initialize();

    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize();

    TextureManager* instance = TextureManager::GetInstance();
    instance->Initialize();

    Sprite::StaticInitialize(WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    ModelManager::GetInstance()->Initialize();

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();

    input_ = Input::GetInstance();
    input_->Initilize(winApp_);

    particleManager_ = ParticleSystem::GetInstance();
    particleManager_->Initialize();

    Time::Initialize();

    audio_ = Audio::GetInstance();
    audio_->Initialize();

    AudioSystem::GetInstance()->Initialize();

    gameTime_ = GameTime::GetInstance();
    gameTime_->Initialize();


    collisionManager_ = CollisionManager::GetInstance();
    collisionManager_->Initialize();

    sceneManager_ = SceneManager::GetInstance();



#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->Initialize();
#endif

}

void Framework::Update()
{
    if (winApp_->ProcessMessage())
    {
        endRequest_ = true;
    }

    // フレーム始め
    imguiManager_->Begin();
    ImGuiDebugManager::GetInstance()->ShowDebugWindow();

    Time::Update();
    gameTime_->Update();

    input_->Update();

}

void Framework::PreDraw()
{
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
    collisionManager_->Finalize();
    sceneManager_->Finalize();

    imguiManager_->Finalize();
    delete imguiManager_;

    winApp_->Finalize();
}

