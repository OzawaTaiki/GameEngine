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
    oModel_->Initialize("bunny.gltf");
    oModel_->translate_.x = 3;

    oModel2_ = std::make_unique<ObjectModel>("cube");
    oModel2_->Initialize("Cube/Cube.obj");
    oModel2_->translate_.x = -3;

    aModel_ = std::make_unique<ObjectModel>("sample");
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    plane_ = std::make_unique<ObjectModel>("plane2");
    plane_->Initialize("Tile/Tile.gltf");
    plane_->GetUVTransform().SetScale({ 100,100 });

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

    if (ImGui::Button("rot"))
    {
        aModel_->ChangeAnimation("RotateAnim", 0.5f,true);
    }

    if (ImGui::Button("scale"))
    {
        aModel_->ChangeAnimation("ScaleAnim", 0.5f);
    }

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
    oModel2_->Update();
    aModel_->Update();
    plane_->Update();
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
    ModelManager::GetInstance()->PreDrawForObjectModel();

    oModel_->Draw(&SceneCamera_, { 1,1,1,1 });
    oModel2_->Draw(&SceneCamera_, { 1,1,1,1 });
    plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    Sprite::PreDraw();
    sprite_->Draw();


    //button_->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);

}

void SampleScene::DrawShadow()
{
    PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_ShadowMap);
    PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_ShadowMap);

    oModel_->DrawShadow(&SceneCamera_, 0);
    oModel2_->DrawShadow(&SceneCamera_, 1);
    aModel_->DrawShadow(&SceneCamera_, 2);

    // depthtextureとrendertextreuからIDを指定して影の輪郭を取得
    // https://claude.ai/chat/01808d8d-c6f8-49d8-a17b-bd4981ce2684
    // その陰からメッシュを作成して高さを与えて描画
}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
