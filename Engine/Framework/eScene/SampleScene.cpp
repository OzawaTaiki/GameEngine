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


    Quaternion rotation0 = Quaternion::MakeRotateAxisAngleQuaternion({ 0.71f, 0.71f, 0.0f }, 0.3f);
    Quaternion rotation1 = Quaternion::MakeRotateAxisAngleQuaternion({ 0.71f, 0.0f, 0.71f }, 3.141592f);

    Quaternion interpolate0 = Slerp(rotation0, rotation1, 0.0f);
    Quaternion interpolate1 = Slerp(rotation0, rotation1, 0.3f);
    Quaternion interpolate2 = Slerp(rotation0, rotation1, 0.5f);
    Quaternion interpolate3 = Slerp(rotation0, rotation1, 0.7f);
    Quaternion interpolate4 = Slerp(rotation0, rotation1, 1.0f);


    interpolate0.ShowData("interpolate0 :Slerp(q1,q2,0.0f)",false);
    interpolate1.ShowData("interpolate1 :Slerp(q1,q2,0.3f)", false);
    interpolate2.ShowData("interpolate2 :Slerp(q1,q2,0.5f)", false);
    interpolate3.ShowData("interpolate3 :Slerp(q1,q2,0.7f)", false);
    interpolate4.ShowData("interpolate4 :Slerp(q1,q2,1.0f)", false);
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
