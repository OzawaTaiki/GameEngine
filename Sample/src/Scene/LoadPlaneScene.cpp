#include "LoadPlaneScene.h"

void LoadPlaneScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();


    plane_obj_ = std::make_unique<ObjectModel>();
    plane_obj_->Initialize(objFileName_);

    plane_obj_->translate_ = { 2,0,0 };

    plane_gltf_ = std::make_unique<ObjectModel>();
    plane_gltf_->Initialize(gltfFileName_);

    plane_gltf_->translate_ = { -2,0,0 };

}

void LoadPlaneScene::Update()
{
#ifdef _DEBUG

    ImGui::SeparatorText(".obj");
    ImGui::InputText("objFileName", objFileName_, 128);
    if (ImGui::Button("Load##obj"))
    {
        plane_obj_->SetModel(objFileName_);
    }

    ImGui::SeparatorText(".gltf");
    ImGui::InputText("gltfFileName", gltfFileName_, 128);
    if (ImGui::Button("Load##gltf"))
    {
        plane_gltf_->SetModel(gltfFileName_);
    }



#endif // _DEBUG


    plane_obj_->Update();
    plane_gltf_->Update();
}

void LoadPlaneScene::Draw()
{
    plane_obj_->Draw(&SceneCamera_, { 1,1,1,1 });
    plane_gltf_->Draw(&SceneCamera_, { 1,1,1,1 });
}
