#include "SampleFramework.h"

#include "SampleScene.h"
#include <Features/Scene/ParticleTestScene.h>
#include <Features/PostEffects/DepthBasedOutLine.h>
#include <Features/PostEffects/Dissolve.h>
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
    PSOManager::GetInstance()->CreatePSOForPostEffect("RadialBlur", L"RadialBlur.hlsl");


    DepthBasedOutLine::GetInstance()->Initialize();
    Dissolve::GetInstance()->Initialize();

    sceneManager_->SetSceneFactory(new SceneFactory());
    particleManager_->SetModifierFactory(new ParticleModifierFactory());

    // 最初のシーンで初期化
    sceneManager_->Initialize("Sample");

    currentTex_ = "default";
    redertextureName_["default"] = "postEffect";
    redertextureName_["postEffect"] = "default";
}

void SampleFramework::Update()
{
    Framework::Update();

    rtvManager_->ClearAllRenderTarget();


    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい

    //=============================
    RenderUI();
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

    currentTex_ = "default";

    for (std::string& eff : activeEffects) {
        rtvManager_->SetRenderTexture(redertextureName_[currentTex_]);

        if (eff == "BoxFilter") {
            // boxfilterの処理
             PSOManager::GetInstance()->SetPSOForPostEffect("BoxFilter");
        }
        else if (eff == "GrayScale") {
            // grayscaleの処理
             PSOManager::GetInstance()->SetPSOForPostEffect("GrayScale");
        }
        else if (eff == "Vignette") {
            // vignetteの処理
             PSOManager::GetInstance()->SetPSOForPostEffect("Vignette");
        }
        else if (eff == "Gauss") {
            // gaussの処理
             PSOManager::GetInstance()->SetPSOForPostEffect("Gauss");
        }
        else if (eff == "LuminanceBasedOutline") {
            // luminanceBasedOutlineの処理
             PSOManager::GetInstance()->SetPSOForPostEffect("LuminanceBasedOutline");
        }
        else if (eff == "DepthBasedOutline") {
            // depthBasedOutlineの処理
             DepthBasedOutLine::GetInstance()->Set("default");
        }
        else if (eff == "RadialBlur") {
            // radialBlurの処理
            PSOManager::GetInstance()->SetPSOForPostEffect("RadialBlur");
        }
        else if (eff == "Dissolve") {
            // Dissolveの処理
            Dissolve::GetInstance()->Set();
        }
        else {
            // 未知のエフェクト名の場合は何もしない
            continue;
        }
        rtvManager_->DrawRenderTexture(currentTex_);
        currentTex_ = redertextureName_[currentTex_];
    }

    dxCommon_->PreDraw();
    // スワップチェインに戻す
    rtvManager_->SetSwapChainRenderTexture(dxCommon_->GetSwapChain());

    PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_OffScreen);
    PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_OffScreen);

    // レンダーテクスチャを描画
    rtvManager_->DrawRenderTexture(currentTex_);

    Framework::PostDraw();

    // 後にupdateに
    sceneManager_->ChangeScene();

}

void SampleFramework::Finalize()
{
    Framework::Finalize();
}

void SampleFramework::RenderUI()
{
    // 利用可能なエフェクトリスト
    static const char* availableEffects[] = {
        "BoxFilter", "GrayScale", "Vignette",
        "Gauss", "LuminanceBasedOutline",
        "DepthBasedOutline","RadialBlur","Dissolve"
    };
    static const int numAvailableEffects = IM_ARRAYSIZE(availableEffects);


    // 現在コンボボックスで選択中のエフェクトインデックス
    static int currentItem = 0;

    // ウィンドウを作成
    ImGui::Begin("Post Effect Manager");

    // エフェクト選択用コンボボックス
    ImGui::Text("Select Effect:");
    ImGui::Combo("##EffectCombo", &currentItem, availableEffects, numAvailableEffects);

    // エフェクト追加ボタン
    if (ImGui::Button("Apply Effect")) {
        // 選択されたエフェクトをアクティブリストに追加
        std::string selectedEffect = availableEffects[currentItem];
        activeEffects.push_back(selectedEffect);
    }

    ImGui::SameLine();

    // エフェクトリストをクリアするボタン
    if (ImGui::Button("Clear All")) {
        activeEffects.clear();
    }

    // 現在アクティブなエフェクトの表示
    ImGui::Separator();
    ImGui::Text("Active Effects (in order):");

    // 削除するエフェクトのインデックス (-1は削除なし)
    static int effectToRemove = -1;

    // アクティブエフェクトのリスト表示と管理
    for (int i = 0; i < activeEffects.size(); i++) {
        ImGui::PushID(i);

        // エフェクト名の表示
        ImGui::BulletText("%d: %s", i + 1, activeEffects[i].c_str());

        ImGui::SameLine();

        // エフェクトを上に移動するボタン
        if (i > 0 && ImGui::Button("up")) {
            std::swap(activeEffects[i], activeEffects[i - 1]);
        }

        ImGui::SameLine();

        // エフェクトを下に移動するボタン
        if (i < activeEffects.size() - 1 && ImGui::Button("dowm")) {
            std::swap(activeEffects[i], activeEffects[i + 1]);
        }

        ImGui::SameLine();

        // エフェクトを削除するボタン
        if (ImGui::Button("X")) {
            effectToRemove = i;
        }

        if (activeEffects[i] == "Dissolve")
        {
            static float threshold = 0.0f;
            static Vector3 maskColor = { 0.0f, 0.0f, 0.0f };
            static bool enableEdgeColor = false;
            static Vector3 edgeColor = { 0.0f, 0.0f, 0.0f };
            static Vector2 edgeDitectRange = { 0.0f, 0.3f };

            ImGui::SliderFloat("Dissolve Threshold", &threshold, 0.0f, 1.0f);
            ImGui::ColorEdit3("Dissolve Mask Color", (float*)&maskColor);
            ImGui::Checkbox("Enable Edge Color", &enableEdgeColor);
            if (enableEdgeColor)
            {
                ImGui::ColorEdit3("Edge Color", (float*)&edgeColor);
                ImGui::SliderFloat2("Edge Ditect Range", (float*)&edgeDitectRange, 0.0f, 1.0f);
            }

            Dissolve::GetInstance()->SetThreshold(threshold);
            Dissolve::GetInstance()->SetMaskColor(maskColor);
            Dissolve::GetInstance()->SetEnableEdgeColor(enableEdgeColor);
            Dissolve::GetInstance()->SetEdgeColor(edgeColor);
            Dissolve::GetInstance()->SetEdgeDitectRange(edgeDitectRange);
        }
        else if (activeEffects[i] == "DepthBasedOutline")
        {
            //DepthBasedOutLine::GetInstance()->Set("default");
        }

        ImGui::PopID();
    }

    // 削除処理（forループの外で行う）
    if (effectToRemove >= 0) {
        activeEffects.erase(activeEffects.begin() + effectToRemove);
        effectToRemove = -1;
    }

    ImGui::End();
}
