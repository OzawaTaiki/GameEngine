#include "SampleScene.h"

#include "SceneManager.h"
#include <UI/ImGuiManager/ImGuiManager.h>

#include <Physics/Math/Quaternion.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Matrix4x4.h>
#include <Physics/Math/MyLib.h>

std::unique_ptr<BaseScene>SampleScene::Create()
{
    return std::make_unique<SampleScene>();
}

SampleScene::~SampleScene()
{
}

void SampleScene::Initialize()
{
}

void SampleScene::Update()
{

    Vector3 from0 = Normalize({ 1,0.7f,.05f });
    Vector3 to0 = -from0;
    Vector3 from1 = Normalize({ -0.6f,0.9f,0.2f });
    Vector3 to1 = Normalize({ 0.4f,0.7f,-0.5f });

    Matrix4x4 rot0 = DirectionToDirection({ 1,0,0 }, { -1,0,0 });
    Matrix4x4 rot1 = DirectionToDirection(from0, to0);
    Matrix4x4 rot2 = DirectionToDirection(from1, to1);

    rot0.ShowData("rot0", false);
    rot1.ShowData("rot1", false);
    rot2.ShowData("rot2", false);


}

void SampleScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
