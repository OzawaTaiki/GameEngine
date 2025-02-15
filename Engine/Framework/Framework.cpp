#include <Framework/Framework.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Light/System/LightingSystem.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
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
    rtvManager_->CreateRenderTexture("default", WinApp::kWindowWidth_, WinApp::kWindowHeight_, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(1.0f, 0.0f, 0.0f, 1.0f));


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

    gameTime_ = GameTime::GetInstance();
    gameTime_->Initialize();


    sceneManager_ = SceneManager::GetInstance();

    collisionManager_ = CollisionManager::GetInstance();

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
    rtvManager_->SetRenderTexture(2, 0);
    srvManager_->PreDraw();
}

void Framework::PostDraw()
{
    dxCommon_->PreDraw();
    rtvManager_->SetSwapChainRenderTexture(dxCommon_->GetSwapChain(), 0);
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

