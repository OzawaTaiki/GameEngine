#pragma once
#include "Vector3.h"
#include "Vector4.h"
struct ParticleInitParam
{
    // 有効時間
    float lifeTime = 1.0f;

    // サイズ
    Vector3 currentSize = { 1,1,1 };
    bool changeSize = false;
    Vector3 startSize = { 1,1,1 };
    Vector3 endSize = { 1,1,1 };

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

    // 色
    Vector4 currentColor = { 1,1,1,1 };
    bool changeColor = false;
    Vector4 startColor = { 1,1,1,1 };
    Vector4 endColor = { 1,1,1,1 };

    // フェード
    bool isFade = false;
    float fadeRatio = 0.0f;
};