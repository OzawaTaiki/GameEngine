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
    Vector3 axis = { 1.0f,0.4f,-0.2f };
    Quaternion rotation = Quaternion::MakeRotateAxisAngleQuaternion(axis.Normalize(), 0.45f);
    Vector3 pointY = { 2.1f,-0.9f,1.3f };
    Matrix4x4 rotateMat = rotation.ToMatrix();
    Vector3 rotateByQuterion = rotation.RotateVector(pointY);
    Vector3 rotateByMatrix = Transform(pointY, rotateMat);

    rotation.ShowData("rotation",false);
    rotateMat.ShowData("rotateMat", false);
    rotateByQuterion.ShowData("rotateByQuterion", false);
    rotateByMatrix.ShowData("rotateByMatrix  ", false);

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
