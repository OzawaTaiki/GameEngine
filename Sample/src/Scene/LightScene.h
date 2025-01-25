#pragma once

#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Light/Group/LightGroup.h>

#include <memory>
#include <vector>



class LightScene : public BaseScene
{
public:
    LightScene()=default;
    ~LightScene();
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:


    Camera SceneCamera_ = {};

    LightGroup lights_{};

    DirectionalLight directionalLight_ = {};

    std::vector<PointLight> pointLights_;
    std::vector<SpotLight> spotLights_;

    bool directionalLightEnable_ = true;
    bool pointLightEnable_ = true;
    bool spotLightEnable_ = true;

    std::unique_ptr<ObjectModel> terrain_ = nullptr;

};