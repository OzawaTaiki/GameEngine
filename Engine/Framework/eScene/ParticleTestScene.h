#pragma once
#include "BaseScene.h"

#include <Framework/Camera/Camera.h>
#include <Framework/Camera/DebugCamera.h>
#include <Rendering/Model/ObjectModel.h>

#include <Rendering/LineDrawer/LineDrawer.h>

#include <Framework/Particle/ParticleManager.h>
#include <Framework/Particle/ParticleEmitters.h>
#include <Framework/Particle/Effect.h>

#include <list>
#include <memory>

class ParticleTestScene : public BaseScene
{
public:
     ~ParticleTestScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

    //--------------- パーティクル関連変数----------------//

    // エフェクトの最大数
    static const uint32_t kMaxEffectNum = 30;
    // Effectのリスト
    std::list<Effect> effects_ = {};
    // 追加するエフェクトの名前
    std::string addEffectName_ = "";
    // 選択されたエフェクトのイテレータ
    std::list<Effect>::iterator selectedEffect_ = effects_.begin();




    // エミッターの最大数
    static const uint32_t kMaxEmitterNum = 30;
    // 選択されたエミッター
    //uint32_t selectedEmitter_ = 0;
    // エミッターのリスト
    std::list<ParticleEmitter*> emitters_ = {};
    // 追加するエミッターの名前
    std::string addEmitterName_ = "";
    Camera SceneCamera_ = {};


    // シーン関連
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    LineDrawer* lineDrawer_ = nullptr;

    std::unique_ptr<ObjectModel> plane_ = nullptr;

    //ConfigManager* pConfigManager_ = nullptr;


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
