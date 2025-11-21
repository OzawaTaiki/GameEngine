#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Core/DXCommon/DXCommon.h>
#include <System/Input/Input.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Random/RandomGenerator.h>

#include <Debug/ImGuiDebugManager.h>


void Camera::Initialize(CameraType _cameraType, const Vector2& _winSize)
{
    cameraType_ = _cameraType;
    winSize_ = _winSize;
    aspectRatio_ = winSize_.x / winSize_.y;

    if (cameraType_ == CameraType::Orthographic)
    {
        LeftTop_={ 0,0 };
        RightBottom_ = winSize_;
        //LeftTop_ = { winSize_.x * -0.5f, winSize_.y * 0.5f};
        //RightBottom_ = { winSize_.x * 0.5f, winSize_.y * -0.5f };
    }

    Map();
    UpdateMatrix();


    gameTime_ = GameTime::GetInstance();
}

void Camera::Update()
{
    if (shaking_)
    {
        UpdateShake();
    }

    //Vector3 move;
    //Input::GetInstance()->GetMove(move, 0.1f);
    //Vector3 rot;
    //Input::GetInstance()->GetRotate(rot);

    //Matrix4x4 matRot = MakeRotateMatrix(rotate_ + rot);
    //Vector3 rotVelo = TransformNormal(move, matRot);

    //translate_ += rotVelo;
}

void Camera::Draw()
{
}

Vector2 Camera::WotldToScreen(const Vector3& _worldPos) const
{
    Vector3 temp = Transform(_worldPos, matViewProjection_);
    temp = Transform(temp, MakeViewportMatrix(0, 0, winSize_.x, winSize_.y, 0.0f, 1.0f));

    return Vector2(temp.x, temp.y); // x, y座標を返す

}


void Camera::TransferData()
{
    // ビュー行列の逆行列を計算してカメラの位置を抽出
    //Matrix4x4 iView = Inverse(matView_);
    //translate_ = { iView.m[3][0], iView.m[3][1], iView.m[3][2] };

    // ビュー・プロジェクション行列を計算
    matViewProjection_ = matView_ * matProjection_;

    // 定数バッファへのデータ転送
    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;
}

void Camera::UpdateMatrix()
{
    matWorld_ = MakeAffineMatrix(scale_, rotate_, translate_ + shakeOffset_);
    matView_ = Inverse(matWorld_);
    //translate_ = { 0,500,0 };
    //matView_ = LoolAt(translate_, { 0,0,0 }, { 1,0,0 });
    switch (cameraType_)
    {
    case CameraType::Perspective:
        matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
        break;
    case CameraType::Orthographic:
        matProjection_ = MakeOrthographicMatrix(LeftTop_.x, LeftTop_.y, RightBottom_.x, RightBottom_.y, nearClip_, farClip_);
        break;
    default:
        break;
    }
    //matProjection_ = MakeOrthographicMatrix(0, 0, 1280, 720, 0.1f, 1000.0f);
    matViewProjection_ = matView_ * matProjection_;


    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;

}

void Camera::Shake(float _time, const Vector2& _RangeMin, const Vector2& _RangeMax)
{
    shaking_ = true;
    shakeTime_ = _time;
    shakeTimer_ = 0.0f;
    shakeRangeMin_ = _RangeMin;
    shakeRangeMax_ = _RangeMax;
}

void Camera::ShakeParametaerSettingFromImGui()
{
#ifdef _DEBUG

    ImGui::SeparatorText("Shake");
    ImGui::DragFloat("time", &shakeTime_, 0.01f);
    ImGui::DragFloat2("rangeMin", &shakeRangeMin_.x, 0.01f);
    ImGui::DragFloat2("rangeMax", &shakeRangeMax_.x, 0.01f);
    if (ImGui::Button("Shake"))
    {
        Shake(shakeTime_, shakeRangeMin_, shakeRangeMax_);
    }
#endif // _DEBUG
}

void Camera::QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const
{
    _cmdList->SetGraphicsRootConstantBufferView(_index, resource_->GetGPUVirtualAddress());
}

void Camera::ImGui()
{
#ifdef _DEBUG

    if (ImGuiDebugManager::GetInstance()->Begin("Camera"))
    {
        ImGui::PushID(this);
        ImGui::DragFloat3("translate", &translate_.x, 0.01f);
        ImGui::DragFloat3("rotate", &rotate_.x, 0.01f);

        ImGui::DragFloat("fovY", &fovY_, 0.01f, 1.0f, 179.0f);
        ImGui::DragFloat("nearClip", &nearClip_, 0.01f, 0.01f, 100.0f);
        ImGui::DragFloat("farClip", &farClip_, 1.0f, 10.0f, 10000.0f);
        ImGui::DragFloat("aspectRatio", &aspectRatio_, 0.01f, 0.1f, 4.0f);


        ShakeParametaerSettingFromImGui();

        ImGui::PopID();
        ImGui::End();
    }
#endif // _DEBUG
}

void Camera::UpdateShake()
{
    shakeTimer_ += gameTime_->GetChannel(timeChannel_).GetDeltaTime<float>();
    if (shakeTimer_ >= shakeTime_)
    {
        shaking_ = false;
        shakeTimer_ = 0.0f;
        shakeOffset_ = { 0.0f,0.0f,0.0f };
    }
    else
    {
        shakeOffset_ = RandomGenerator::GetInstance()->GetRandValue(shakeRangeMin_, shakeRangeMax_);
    }
}

void Camera::Map()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferDate));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}

Matrix4x4 Camera::LoolAt(const Vector3& _eye, const Vector3& _at, const Vector3& _up)
{
    Vector3 zaxis = (_at - _eye).Normalize();
    Vector3 xaxis = _up.Cross(zaxis).Normalize();
    if (xaxis.Length() == 0)
    {
        xaxis = { 1.0f,0.0f,0.0f };
    }
    Vector3 yaxis = zaxis.Cross(xaxis);
    if (yaxis.Length() == 0)
    {
        yaxis = { 0.0f,1.0f,0.0f };
    }

    Matrix4x4 result =
    {
        {
            {xaxis.x, yaxis.x, zaxis.x, 0.0f},
            {xaxis.y, yaxis.y, zaxis.y, 0.0f},
            {xaxis.z, yaxis.z, zaxis.z, 0.0f},
            { -xaxis.Dot(_eye), -yaxis.Dot(_eye), -zaxis.Dot(_eye), 1.0f }
        }
    };

    return result;
}
