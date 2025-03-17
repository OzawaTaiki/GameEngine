#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImguITools.h>


SampleScene::~SampleScene()
{
}

void SampleScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();


    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    oModel_ = std::make_unique<ObjectModel>("plane");
    oModel_->Initialize("Plane/Plane.gltf");
    oModel_->translate_.x = 3;

    aModel_ = std::make_unique<AnimationModel>("sample");
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);

    lights_ = std::make_unique<LightGroup>();
    lights_->Initialize();

    colors.push_back({ 0.0f,Vector4(1,0,0,1) });
    colors.push_back({ 1.0f,Vector4(0,0,1,1) });
    colors.push_back({ 0.5f,Vector4(0,1,0,1) });
    colors.push_back({ 0.1f,Vector4(0,1,0,1) });
    colors.push_back({ 0.532f,Vector4(0,1,0,1) });
    colors.push_back({ 0.12f,Vector4(1,1,0,1) });

    sequence_ = std::make_unique<AnimationSequence>("test");
    sequence_->Initialize("Resources/Data/");

}

void SampleScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

    ImGuiTool::TimeLine("TimeLine", sequence_.get());
    ImGuiTool::GradientEditor("GradientEditor", colors);

    lights_->DrawDebugWindow();

    static bool play = false;
    if (ImGui::Button("Play"))
    {
        //play = !play;
        sequence_->Save();
    }
    if (play)
        oModel_->translate_ = sequence_->GetValue<Vector3>("a");

#endif // _DEBUG
    LightingSystem::GetInstance()->SetLightGroup(lights_.get());


    oModel_->Update();
    aModel_->Update();
    sprite_->Update();

    if (input_->IsKeyTriggered(DIK_TAB))
    {
        SceneManager::GetInstance()->ReserveScene("ParticleTest");
    }

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        ParticleManager::GetInstance()->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
        ParticleManager::GetInstance()->Update(SceneCamera_.rotate_);
    }

}

void SampleScene::Draw()
{

    oModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    Sprite::PreDraw();
    sprite_->Draw();


    //button_->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);

}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
