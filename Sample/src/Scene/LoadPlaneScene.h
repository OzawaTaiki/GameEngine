#pragma once


#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Model/ObjectModel.h>


class LoadPlaneScene : public BaseScene
{
public:
    LoadPlaneScene() = default;
    ~LoadPlaneScene() override {};

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

    Camera SceneCamera_ = {};
    std::unique_ptr<ObjectModel> plane_obj_ = nullptr;
    std::unique_ptr<ObjectModel> plane_gltf_ = nullptr;


    char objFileName_[128] = "plane/plane.obj";
    char gltfFileName_[128] = "plane/plane.gltf";



};
