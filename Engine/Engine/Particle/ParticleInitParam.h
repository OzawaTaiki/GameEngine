#pragma once
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

template <class T>
struct ValueTransition
{
    T current;
    T start;
    T end;
    bool isChange;

    std::list <T> keys;

    // イージング関数 ポインタで所持
    float (*pEasingFunc)(float);

};

struct ParticleInitParam
{
    // 有効時間
    float lifeTime = 1.0f;

    // サイズ
    Vector3 currentSize = { 1,1,1 };
    bool changeSize = false;
    Vector3 startSize = { 1,1,1 };
    Vector3 endSize = { 1,1,1 };

    // TODO : 回転も変更できるようにする あとまわし
    // 回転
    Vector3 rotate = { 0,0,0 };
    // 位置
    Vector3 position = { 0,0,0 };

    // スピード
    float speed = 0.0f;
    // 方向
    Vector3 direction = { 0,0,0 };
    // 加速度，重力
    Vector3 acceleration = { 0,0,0 };
    // 減速係数
    float deceleration = 0.0f;

    // 色
    Vector4 currentColor = { 1,1,1,1 };
    bool changeColor = false;
    Vector4 startColor = { 1,1,1,1 };
    Vector4 endColor = { 1,1,1,1 };

    // フェード
    bool isFade = false;
    float fadeRatio = 0.0f;

    Matrix4x4 directionMatrix = Matrix4x4::Identity();
};