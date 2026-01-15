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

#include <Features/UI/Element/UIButtonElement.h>
#include <Features/UI/Element/UISliderElement.h>
#include <Features/UI/Element/UIImageElement.h>
#include <Features/UI/Element/UITextElement.h>




class DeveScene : public Engine::BaseScene
{
public:

    ~DeveScene() override;

    void Initialize(Engine::SceneData* _sceneData) override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

private:
    // -----------------------------
    // シーン関連 基本セット

    // カメラ
    Engine::Camera SceneCamera_ = {};

    // デバッグカメラ
    Engine::DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    // ライン描画
    Engine::LineDrawer* lineDrawer_ = nullptr;
    // 入力
    Engine::Input* input_ = nullptr;
    // パーティクル
    Engine::ParticleSystem* particleManager_ = nullptr;
    // ライト
    std::shared_ptr<Engine::LightGroup> lights_;

    Engine::TextGenerator text_;

    //------------------------------
    // シーン固有


    std::unique_ptr<Engine::ObjectModel> ground_ = nullptr;

    uint32_t groundTextureHandle_ = 0;

    Engine::Vector4 drawColor_ = { 1,1,1,1 }; // 描画色

    std::unique_ptr<Engine::Sprite> sprite_ = nullptr;


    std::unique_ptr<Engine::ParticleEmitter> emitter_ = nullptr;

    std::shared_ptr<Engine::SoundInstance> soundInstance_ = nullptr;
    std::shared_ptr<Engine::VoiceInstance> voiceInstance_ = nullptr;

    std::unique_ptr<Engine::SkyBox> skyBox_ = nullptr;

    // UIRenderComponent テスト
    std::unique_ptr<Engine::UIElement> testTextElement_ = nullptr;
    std::unique_ptr<Engine::UIElement> testSpriteElement_ = nullptr;
    std::unique_ptr<Engine::UIElement> testBackgroundElement_ = nullptr;

    // UIButtonElement テスト
    std::unique_ptr<Engine::UIButtonElement> testButton1_ = nullptr;
    std::unique_ptr<Engine::UIButtonElement> testButton2_ = nullptr;
    std::unique_ptr<Engine::UIButtonElement> testButton3_ = nullptr;
    std::unique_ptr<Engine::UIButtonElement> testButton4_ = nullptr;

    // UISliderElement テスト
    std::unique_ptr<Engine::UISliderElement> testSlider1_ = nullptr;

    // UIImageElement テスト
    std::unique_ptr<Engine::UIImageElement> testImage1_ = nullptr;

    // UITextElement テスト
    std::unique_ptr<Engine::UITextElement> testText1_ = nullptr;



#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
