#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Debug/ImguITools.h>
#include <Features/Model/Primitive/Ring.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/PostEffects/DepthBasedOutLine.h>

#include <Features/Effect/Emitter/ParticleEmitter.h>


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
    oModel_->Initialize("plane/plane.gltf");
    oModel_->translate_.x = 3;

    oModel2_ = std::make_unique<ObjectModel>("cube");
    oModel2_->Initialize("Cube/Cube.obj");
    oModel2_->translate_.x = -3;

    aModel_ = std::make_unique<ObjectModel>("sample");
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    plane_ = std::make_unique<ObjectModel>("ground");
    plane_->Initialize("terrain.obj");
    plane_->GetUVTransform().SetScale({ 100,100 });

    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);

    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();

    sequence_ = std::make_unique<AnimationSequence>("test");
    sequence_->Initialize("Resources/Data/");

    Cylinder* cylinder = new Cylinder(1.0f, 2.0f,1.0f);
    cylinder->SetDivide(32);
    cylinder->SetEndAngle(3.14f);
    cylinder->SetLoop(true);

    Triangle* triangle = new Triangle();
    triangle->SetVertices(
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(1.0f, -1.0f, 0.0f),
        Vector3(-1.0f, -1.0f, 0.0f)
    );
    triangle->SetNormal(Vector3(0.0f, 1.0f, 0.0f).Normalize());

    Plane* plane = new Plane();
    plane->SetNormal(Vector3(1.0f, 1.0f, 0.0f).Normalize());


    test_ = std::make_unique<ObjectModel>("triangle");
    test_->Initialize(triangle->Generate("triangle"));

    //emitter_ = std::make_unique<ParticleEmitter>();
    //emitter_->Initialize("test");

    DepthBasedOutLine::GetInstance()->SetCamera(&SceneCamera_);

    ParticleSystem::GetInstance()->SetCamera(&SceneCamera_);
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
        aModel_->ChangeAnimation("RotateAnim", 0.5f, true);
    }

    if (ImGui::Button("scale"))
    {
        aModel_->ChangeAnimation("ScaleAnim", 0.5f);
    }

    ImGuiTool::TimeLine("TimeLine", sequence_.get());

    lights_->ImGui();

    static bool play = false;
    ImGui::Checkbox("Play", &play);

    if (play)
        testColor_= sequence_->GetValue<Vector4>("color");

    //emitter_->ShowDebugWindow();


#endif // _DEBUG
    LightingSystem::GetInstance()->SetActiveGroup(lights_);


    test_->Update();
    oModel_->Update();
    oModel2_->Update();
    aModel_->Update();
    plane_->Update();
    sprite_->Update();

    if (input_->IsKeyTriggered(DIK_TAB))
    {
        //SceneManager::GetInstance()->ReserveScene("ParticleTest");
    }

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        //ParticleManager::GetInstance()->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
    }

    ParticleSystem::GetInstance()->Update();
    CollisionManager::GetInstance()->Update();
}

void SampleScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();

    oModel_->Draw(&SceneCamera_, testColor_);
    //oModel2_->Draw(&SceneCamera_, { 1,1,1,1 });
    plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    //aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    test_->Draw(&SceneCamera_, 2, { 1,1,1,1 });
    Sprite::PreDraw();
    sprite_->Draw();


    //button_->Draw();

    ParticleSystem::GetInstance()->DrawParticles();

}

void SampleScene::DrawShadow()
{

    //oModel_->DrawShadow(&SceneCamera_, 0);
    //oModel2_->DrawShadow(&SceneCamera_, 1);
    //aModel_->DrawShadow(&SceneCamera_, 2);

}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
