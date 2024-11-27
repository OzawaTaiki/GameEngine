#pragma once

#include "Model.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "ParticleInitParam.h"

class Particle
{
public:

    Particle() = default;
    ~Particle() = default;

    void Initialize(float _lifeTime,
                    const Vector3& _size,
                    const Vector3& _rotate,
                    const Vector3& _pos,
                    const Vector4& _color,
                    float _speed,
                    const Vector3& _direction,
                    const Vector3& _acceleration,
                    bool _fade,
                    float _faderatio = 0.0f
    );

    void Initialize(const ParticleInitParam& _param);
    void Update();
    void Draw();

    bool IsAlive() const { return isAlive_; }

    Matrix4x4 GetWorldMatrix() const { return matWorld_; }
    Vector3 GetPosition() const { return translate_; }
    Vector3 GetRotation() const { return rotation_; }
    Vector3 GetScale() const { return scale_; }
    Vector4 GetColor() const { return color_; }

private:

    Matrix4x4 matWorld_;

    bool isAlive_;
    float lifeTime_;
    float currentTime_ = 0;

    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    bool isChangeScale_ = false;
    Vector3 minScale_ = { 1.0f,1.0f ,1.0f };
    Vector3 maxScale_ = { 1.0f,1.0f ,1.0f };

    Vector3 rotation_ = { 0.0f,0.0f ,0.0f };
    Vector3 translate_ = { 0.0f,0.0f ,0.0f };

    Vector4 color_ = { 1.0f,1.0f ,1.0f,1.0f };
    bool isChangeColor_ = false;
    Vector4 startColor_ = { 1.0f,1.0f,1.0f ,1.0f };
    Vector4 endColor_ = { 1.0f,1.0f ,1.0f,1.0f };

    float speed_;
    Vector3 direction_;
    Vector3 acceleration_;
    Vector3 velocity_;

    // フェードするか否か
    bool isFade_;
    // フェードを始める割合
    float fadeRatio_;


};