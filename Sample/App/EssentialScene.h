#pragma once
#include <Framework/eScene/BaseScene.h>

#include <Framework/Camera/Camera.h>
#include <Framework/Camera/DebugCamera.h>
#include <Rendering/Model/ObjectModel.h>
#include <Systems/Config/ConfigManager.h>
#include <Framework/Particle/ParticleManager.h>
#include <Rendering/LineDrawer/LineDrawer.h>
#include <Systems/Input/Input.h>

#include <Framework/Particle/Particle.h>
#include <vector>

#include <memory>

class EssentialScene :public BaseScene
{
public:

    static std::unique_ptr<BaseScene> Create();

    EssentialScene() = default;
    ~EssentialScene() override;

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

    std::unique_ptr<ObjectModel> plane_ = nullptr;


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};