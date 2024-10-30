#include "Player.h"

void Player::Initialize()
{
    model_ = Model::CreateFromObj("Player/Player.gltf");

    worldTransform_.Initialize();
}

void Player::Update()
{
    worldTransform_.UpdateData();
}

void Player::Draw(const Camera* _camera)
{
    model_->Draw(worldTransform_, _camera);
}
