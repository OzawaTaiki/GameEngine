#pragma once

// 必須
#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>

#include <System/Input/Input.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/Light/Group/LightGroup.h>

#include <memory>

#include <Features/Sprite/Sprite.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Emitter/ParticleEmitter.h>

#include <System/Audio/AudioSystem.h>
#include <System/Audio/SoundInstance.h>
#include <system/Audio/VoiceInstance.h>

#include <Features/Model/SkyBox.h>

#include <Features/TextRenderer/TextGenerator.h>
#include <Features/UI/UIGroup.h>

#include <Features/UI/UIElement.h>
#include <Features/UI/UITextRenderComponent.h>
#include <Features/UI/UISpriteRenderComponent.h>


class DeveScene : public BaseScene
{
public:

    ~DeveScene() override;

    void Initialize(SceneData* _sceneData) override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

private:
    // -----------------------------
    // シーン関連 基本セット

    // カメラ
    Camera SceneCamera_ = {};

    // デバッグカメラ
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    // ライン描画
    LineDrawer* lineDrawer_ = nullptr;
    // 入力
    Input* input_ = nullptr;
    // パーティクル
    ParticleSystem* particleManager_ = nullptr;
    // ライト
    std::shared_ptr<LightGroup> lights_;

    TextGenerator text_;

    //------------------------------
    // シーン固有


    std::unique_ptr<ObjectModel> ground_ = nullptr;

    uint32_t groundTextureHandle_ = 0;

    Vector4 drawColor_ = { 1,1,1,1 }; // 描画色

    std::unique_ptr <Sprite> sprite_ = nullptr;


    std::unique_ptr<ParticleEmitter> emitter_ = nullptr;

    std::shared_ptr<SoundInstance> soundInstance_ = nullptr;
    std::shared_ptr<VoiceInstance> voiceInstance_ = nullptr;

    std::unique_ptr<SkyBox> skyBox_ = nullptr;

    std::unique_ptr<UIGroup> uiGroup_ = nullptr;

    // UIRenderComponent テスト
    std::unique_ptr<UIElement> testTextElement_ = nullptr;
    std::unique_ptr<UIElement> testSpriteElement_ = nullptr;
    std::unique_ptr<UIElement> testBackgroundElement_ = nullptr;



#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
