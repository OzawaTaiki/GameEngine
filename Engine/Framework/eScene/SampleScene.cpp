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

    Quaternion q1 = { 2.0f,3.0f,4.0,1.0f };
    Quaternion q2 = { 1.0f,3.0f,5.0,2.0f };
    Quaternion identity = Quaternion::Identity();
    Quaternion coj = q1.Conjugate();
    Quaternion inv = q1.Inverse();
    Quaternion normal = q1.Normalize();
    Quaternion mul1 = q1 * q2;
    Quaternion mul2 = q2 * q1;

    float norm = q1.Norm();

    identity.ShowData("identity", false);
    coj.ShowData("conjugate", false);
    inv.ShowData("inverse", false);
    normal.ShowData("normalize", false);
    mul1.ShowData("mul1", false);
    mul2.ShowData("mul2", false);

    ImGui::Text("norm : %.2f", norm);

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
