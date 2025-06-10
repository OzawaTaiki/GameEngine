#include "DeveScene.h"

#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <Features/Scene/Manager/SceneManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Model/Primitive/Triangle.h>
#include <Features/Model/Primitive/Plane.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/PostEffects/DepthBasedOutLine.h>

#include <Debug/ImGuiManager.h>


DeveScene::~DeveScene()
{
}

void DeveScene::Initialize(SceneData* _sceneData)
{

    // --------------------------------------------------
    // シーン関連 初期化
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();


    lineDrawer_ = LineDrawer::GetInstance();
    //lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    // パーティクルシステムの初期化
    // カメラのポイントを設定する
    ParticleSystem::GetInstance()->SetCamera(&SceneCamera_);


    // ライトの設定
    // ライトグループの初期化 (関数内でDLは初期化される)
    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();

    // DLを取得して初期化 (任意)
    auto DL = lights_->GetDirectionalLight();
    DL->SetDirection(Vector3(-1.0f, -1.0f, 0.0f).Normalize());

    // ライトの追加(任意)
    // 追加するライトを初期化する
    std::shared_ptr<PointLightComponent> pointLight = std::make_shared<PointLightComponent>();
    pointLight->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    pointLight->SetIntensity(1.0f);
    pointLight->SetRadius(30.0f);
    pointLight->SetPosition({ 0.0f, 5.0f, 0.0f });

    // ライトグループにポイントライトを追加する
    lights_->AddPointLight("pointlight", pointLight);

    // アクティブなライトグループを設定する
    LightingSystem::GetInstance()->SetActiveGroup(lights_);



    // --------------------------------------------------
    // シーン固有の初期化



    // 地面ようのいたポリを生成する
    Plane groundPlane;
    groundPlane.SetSize(Vector2{ 100, 100 });   //サイズは100*100
    groundPlane.SetPivot({ 0,0,0 });        // 基準点は中央
    groundPlane.SetNormal({ 0,1,0 });       //上向き
    groundPlane.SetFlipU(false);            // U軸の反転はなし
    groundPlane.SetFlipV(false);            // V軸の反転はなし

    // 任意の名前で生成する
    groundPlane.Generate("groundPlane");

    // 地面を生成する
    ground_ = std::make_unique<ObjectModel>("ground");
    // 生成した板ポリを使用する (生成時に設定した名前を渡す)
    ground_->Initialize("groundPlane");
    // UV変換を設定する
    ground_->GetUVTransform().SetScale({ 10,10 });

    // 地面のテクスチャを読み込む 描画時に使用する
    groundTextureHandle_ = TextureManager::GetInstance()->Load("tile.png");


    // 2Dスプライトの初期化
    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);


    // 音声データの読み込み
    soundInstance_ = AudioSystem::GetInstance()->Load("Resources/Sounds/Alarm01.wav");

    skyBox_ = std::make_unique<SkyBox>();
    skyBox_->Initialize(30.0f);
    skyBox_->SetTexture("rosendal_plains_2_2k.dds");


}

void DeveScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG

    // デバッグカメラ
    if (Input::GetInstance()->IsKeyTriggered(DIK_F1))
        enableDebugCamera_ = !enableDebugCamera_;


    {
        ImGui::Begin("Engine");
        {
            // サウンドの再生
            if (ImGui::Button("play Sound"))
            {
                if (soundInstance_)
                {
                    // 返り値で VoiceInstanceを受け取る
                    // これを使用して音量やピッチの変更ができる
                    voiceInstance_ = soundInstance_->Play();
                }
            }

            static float volume = 1.0f;
            if (ImGui::DragFloat("Volume", &volume, 0.0f, 1.0f))
            {
                if (voiceInstance_)
                {
                    voiceInstance_->SetVolume(volume);
                }
            }
            if (ImGui::Button("Stop Sound"))
            {
                if (voiceInstance_)
                {
                    voiceInstance_->Stop();
                    voiceInstance_ = nullptr; // VoiceInstanceを解放
                }
            }
        }
        ImGui::End();

        // light調整用
        lights_->ImGui();

    }

#endif // _DEBUG

    // モデルの更新
    ground_->Update();




    // --------------------------------
    // シーン共通更新処理

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
    }

    ParticleSystem::GetInstance()->Update();
    CollisionManager::GetInstance()->Update();
}

void DeveScene::Draw()
{
    // skyBooxの描画
    //skyBox_->Draw(&SceneCamera_);

    // Model描画用のPSO等をセット
    ModelManager::GetInstance()->PreDrawForObjectModel();

    // SkyBoxのキューブマップを描画キューに追加(任意)
    //skyBox_->QueueCmdCubeTexture();

    // groundの描画
    ground_->Draw(&SceneCamera_, groundTextureHandle_, drawColor_);


    // Sprite用のPSO等をセット
    Sprite::PreDraw();
    // スプライトの描画
    sprite_->Draw();


    ParticleSystem::GetInstance()->DrawParticles();
}

void DeveScene::DrawShadow()
{
}
