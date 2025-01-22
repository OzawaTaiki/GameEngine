#pragma once
#include <Framework/eScene/BaseScene.h>
#include <Framework/Camera/Camera.h>
#include <Framework/Camera/DebugCamera.h>
#include <Rendering/Model/ObjectModel.h>
#include <Rendering/Model/AnimationModel.h>

#include <Framework/Particle/ParticleManager.h>
#include <Rendering/LineDrawer/LineDrawer.h>
#include <Systems/Input/Input.h>
#include <Systems/Time/GameTime.h>
#include <UI/UIButton.h>
#include <Rendering/Primitive/Ring.h>
#include <Rendering/Primitive/Ellipse.h>
#include <Rendering/Primitive/Cylinder.h>
#include <Systems/UVTransform/SpriteSheetAnimetion.h>
#include <Systems/UVTransform/UVTransformAnimation.h>

#include <memory>

class SampleScene : public BaseScene
{
public:

     ~SampleScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:
    // シーン関連
    Camera SceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleManager* particleManager_ = nullptr;

    std::unique_ptr<AnimationModel> aModel_ = nullptr;
    std::unique_ptr<ObjectModel> oModel_ = nullptr;

    std::unique_ptr<ObjectModel> plane_ = nullptr;

    std::unique_ptr<LightGroup> lights_;

    std::unique_ptr<UIButton> button_ = nullptr;

    GameTime* gameTime_;

    Ring* ring_;

    EllipseModel* ellipse_;

    Cylinder* cylinder_;

    UVTransformAnimation uvTransformAnimetion_;

    SpriteSheetAnimation spriteSheetAnimetion_;



#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
