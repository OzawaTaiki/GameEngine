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

    Vector3 axis = { 1,1,1 };
    axis = axis.Normalize();
    float angle = 0.44f;
    Matrix4x4 rot = MakeRotateAxisAngle(axis, angle);
    rot.ShowData("Rot", false);

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
