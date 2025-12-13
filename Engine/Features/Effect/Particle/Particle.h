#pragma once

#include <Features/Model/Model.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Effect/ParticleInitParam.h>
#include <Features/Camera/Camera/Camera.h>

class Particle
{
public:

    Particle() = default;
    ~Particle() = default;

    void Initialize(const ParticleInitParam& _param);
    void Update(float _deltaTime);

    bool IsAlive() const { return isAlive_; }
    void SetAlive(bool _isAlive) { isAlive_ = _isAlive; }

    Matrix4x4 GetWorldMatrix() const { return matWorld_; }


    float GetCurrentTime() const { return currentTime_; }

    Vector3 GetPosition() const { return translate_; }
    void SetPosition(const Vector3& _pos) { translate_ = _pos; }

    Vector3 GetRotation() const { return rotation_; }
    void SetRotation(const Vector3& _rot) { rotation_ = _rot; }

    Vector3 GetScale() const { return scale_; }
    void SetScale(const Vector3& _scale) { scale_ = _scale; }

    Vector4 GetColor() const { return color_; }
    void SetColor(const Vector4& _color) { color_ = _color; }

    void SetSpeed(float _speed) { speed_ = _speed; }
    float GetSpeed() const { return speed_; }

    void SetDirection(const Vector3& _dir) { direction_ = _dir; }
    Vector3 GetDirection() const { return direction_; }

    void SetAcceleration(const Vector3& _acc) { acceleration_ = _acc; }
    Vector3 GetAcceleration() const { return acceleration_; }

    void SetLifeTime(float _lifeTime) { lifeTime_ = _lifeTime; }
    float GetLifeTime() const { return lifeTime_; }

    void SetInfiniteLife(bool _isInfinite) { isInfiniteLife_ = _isInfinite; }
    bool IsInfiniteLife() const { return isInfiniteLife_; }

    std::array<bool, 3> GetBillboard() const { return isBillboard_; }
    void SetBillboard(const std::array<bool, 3>& _billboard) { isBillboard_ = _billboard; }



    void ShowDebugWindow();

private:

    ParticleInitParam parameter_;

    bool isAlive_ = true;
    bool isInfiniteLife_ = false;

    float currentTime_ = 0;
    float t_ = 0;

    Vector3 velocity_ = {};
    float lifeTime_ = 1.0f;

    Vector3 translate_ = { 0.0f,0.0f ,0.0f };
    Vector3 rotation_ = { 0.0f,0.0f ,0.0f };
    Vector3 rotationSpeed_ = { 0.0f,0.0f ,0.0f };
    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector4 color_ = { 1.0f,1.0f ,1.0f,1.0f };
    float speed_ = 0.0f;
    Vector3 direction_ = {};
    Vector3 acceleration_ = {};



    std::array<bool, 3> isBillboard_ = { false ,false ,false }; // ビルボード xyz

    Matrix4x4 matWorld_ = Matrix4x4::Identity();



};
