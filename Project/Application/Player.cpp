#include "Player.h"
#include "Input.h"

#include "VectorFunction.h"
#include "MatrixFunction.h"

void Player::Initialize()
{
    model_ = Model::CreateFromObj("Player/Player.gltf");

    worldTransform_.Initialize();

    beam_ = std::make_unique<Beam>();
    beam_->Initialize();
    beam_->SetParent(&worldTransform_);
}

void Player::Update(const Matrix4x4& _matVP)
{
    if (Input::GetInstance()->IsKeyTriggered(DIK_SPACE))
    {
        isBeamActive_ = true;
    }
    else if (Input::GetInstance()->IsKeyReleased(DIK_SPACE))
        isBeamActive_ = false;

    Aim(_matVP);
    if(isBeamActive_)
        beam_->Update();
    worldTransform_.UpdateData();
}

void Player::Draw(const Camera* _camera)
{
    model_->Draw(worldTransform_, _camera);
    if (isBeamActive_)
        beam_->Draw(_camera);
}

void Player::Aim(const Matrix4x4& _matVP)
{
    Vector2 mPos = Input::GetInstance()->GetMousePosition();
    const float distance = 30.0f;

    Vector3 nearPos = Vector3(mPos.x, mPos.y, 0.0f);
    Vector3 farPos = Vector3(mPos.x, mPos.y, 1.0f);

    Matrix4x4 viewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth_, WinApp::kWindowHeight_, 0, 1);
    Matrix4x4 invMatVPV = Inverse(_matVP * viewport);

    nearPos = Transform(nearPos, invMatVPV);
    farPos = Transform(farPos, invMatVPV);

    Vector3 dir = Normalize(farPos - nearPos);

    Vector3 wTarget = nearPos + dir * distance;

    Matrix4x4 invWorld = Inverse(worldTransform_.matWorld_);
    Vector3 target = Transform(wTarget, invWorld);

    beam_->SetTarget(target);

}

#ifdef _DEBUG
#include <imgui.h>
void Player::ImGui()
{
    ImGui::BeginTabBar("Player");
    if(ImGui::BeginTabItem("Player"))
    {
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
#endif // _DEBUG
