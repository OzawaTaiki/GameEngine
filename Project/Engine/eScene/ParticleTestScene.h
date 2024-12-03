#pragma once
#include "BaseScene.h"

#include "Camera/Camera.h"
#include "Camera/DebugCamera.h"
#include "Model/ObjectModel.h"

#include "LineDrawer/LineDrawer.h"


#include "Particle/ParticleManager.h"
#include "Particle/ParticleEmitters.h"
#include "Particle/Effect.h"

#include <list>
#include <memory>

class ParticleTestScene : public BaseScene
{
public:

    static std::unique_ptr<BaseScene> Create();

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


    // シーン関連
    Camera SceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    LineDrawer* lineDrawer_ = nullptr;

    std::unique_ptr<ObjectModel> plane_ = nullptr;

    



#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};