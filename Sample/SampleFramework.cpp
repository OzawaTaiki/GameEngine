#include "SampleFramework.h"

#include "SampleScene.h"
#include <Features/Scene/ParticleTestScene.h>
#include <Features/PostEffects/DepthBasedOutLine.h>
#include "SceneFactory.h"
#include "ParticleModifierFactory.h"

void SampleFramework::Initialize()
{
    Framework::Initialize();


    JsonHub::GetInstance()->Initialize("Resources/Data/");

    rtvManager_->CreateRenderTarget("default", WinApp::kWindowWidth_, WinApp::kWindowHeight_, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.4625f, 0.925f, 0.4625f, 1.0f), true);
    rtvManager_->CreateRenderTarget("postEffect", WinApp::kWindowWidth_, WinApp::kWindowHeight_, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.4625f, 0.925f, 0.4625f, 1.0f), false);
    rtvManager_->CreateRenderTarget("ShadowMap", 4096, 4096, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,  Vector4(1.0f, 1.0f, 1.0f, 1.0f),true);

    PSOManager::GetInstance()->CreatePSOForPostEffect("GrayScale", L"GrayScale.hlsl");
    PSOManager::GetInstance()->CreatePSOForPostEffect("Vignette", L"Vignetting.hlsl");
    PSOManager::GetInstance()->CreatePSOForPostEffect("BoxFilter", L"BoxFilter.hlsl");
    PSOManager::GetInstance()->CreatePSOForPostEffect("Gauss", L"GaussianFilter.hlsl");
    PSOManager::GetInstance()->CreatePSOForPostEffect("LuminanceBasedOutline", L"LuminanceBasedOutline.hlsl");

    DepthBasedOutLine::GetInstance()->Initialize();

    sceneManager_->SetSceneFactory(new SceneFactory());
    particleManager_->SetModifierFactory(new ParticleModifierFactory());

    // 最初のシーンで初期化
    sceneManager_->Initialize("Sample");
}

void SampleFramework::Update()
{
    Framework::Update();

    rtvManager_->ClearAllRenderTarget();


    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい

    //=============================
}

void SampleFramework::Draw()
{
    Framework::PreDraw();

    rtvManager_->SetDepthStencil("ShadowMap");
    sceneManager_->DrawShadow();


    // ========== 描画処理 ==========
    rtvManager_->SetRenderTexture("default");

    sceneManager_->Draw();

    lineDrawer_->Draw();
    //=============================
    static bool is = false;
    ImGui::Checkbox("postEffect", &is);

    if(is)
    {
        rtvManager_->SetRenderTexture("postEffect");
        //PSOManager::GetInstance()->SetPSOForPostEffect("LuminanceBasedOutline");
        DepthBasedOutLine::GetInstance()->Set("default");
        rtvManager_->DrawRenderTexture("default");
    }
    else
    {
        rtvManager_->SetRenderTexture("postEffect");
        PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_OffScreen);
        PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_OffScreen);
        rtvManager_->DrawRenderTexture("default");
    }


    dxCommon_->PreDraw();
    // スワップチェインに戻す
    rtvManager_->SetSwapChainRenderTexture(dxCommon_->GetSwapChain());

    PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_OffScreen);
    PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_OffScreen);

    // レンダーテクスチャを描画
    rtvManager_->DrawRenderTexture("postEffect");

    Framework::PostDraw();

    // 後にupdateに
    sceneManager_->ChangeScene();

}

void SampleFramework::Finalize()
{
    Framework::Finalize();
}
