//#pragma once
//#include <Features/Scene/Interface/BaseScene.h>
//
//#include <Features/Camera/Camera/Camera.h>
//#include <Features/Camera/DebugCamera/DebugCamera.h>
//#include <Features/Model/ObjectModel.h>
//
//#include <Features/LineDrawer/LineDrawer.h>
//
//#include <Features/Effect/Manager/ParticleManager.h>
////#include <Features/Effect/Effect/ParticleEmitters.h>
////#include <Features/Effect/Effect/Effect.h>
//
//#include <list>
//#include <memory>
//
//class ParticleTestScene : public BaseScene
//{
//public:
//     ~ParticleTestScene() override;
//
//    void Initialize() override;
//    void Update() override;
//    void Draw() override;
//    void DrawShadow() override {};
//
//private:
//
//    //--------------- パーティクル関連変数----------------//
//
//    // エフェクトの最大数
//    static const uint32_t kMaxEffectNum = 30;
//    // Effectのリスト
//    std::list<Effect> effects_ = {};
//    // 追加するエフェクトの名前
//    std::string addEffectName_ = "";
//    // 選択されたエフェクトのイテレータ
//    std::list<Effect>::iterator selectedEffect_ = effects_.begin();
//
//
//
//
//    // エミッターの最大数
//    static const uint32_t kMaxEmitterNum = 30;
//    // 選択されたエミッター
//    //uint32_t selectedEmitter_ = 0;
//    // エミッターのリスト
//    std::list<ParticleEmitter*> emitters_ = {};
//    // 追加するエミッターの名前
//    std::string addEmitterName_ = "";
//    Camera SceneCamera_ = {};
//
//
//    // シーン関連
//    DebugCamera debugCamera_ = {};
//    bool enableDebugCamera_ = false;
//
//    LineDrawer* lineDrawer_ = nullptr;
//
//    std::unique_ptr<ObjectModel> plane_ = nullptr;
//
//    std::unique_ptr<LightGroup> lights_;
//
//    //ConfigManager* pConfigManager_ = nullptr;
//
//
//#ifdef _DEBUG
//    void ImGui();
//#endif // _DEBUG
//};


namespace Engine {

} // namespace Engine
