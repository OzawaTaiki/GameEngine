#pragma once
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Physics/Math/Matrix4x4.h>
#include <Physics/Math/MyLib.h>

#include <list>
#include <functional>

template <typename T>
struct KeyFrame
{
    // time で value に変化する
    float time = 1.0f;
    T value = {};

    // イージング関数 ポインタで所持
    std::function<float(float)> pEasingFunc = nullptr;
};

template <typename T>
struct ValueTransition
{
    // 変更するかのフラグ
    bool isChange = false;

    // 変更する値
    std::list<KeyFrame<T>> keys = {};



    // 値の計算
    // イージング関数は後ろの要素を使う
    inline T calculateValue(float _t)
    {
        if (keys.size() == 0)
            return T();
        if (keys.size() == 1)
            return keys.front().value;
        auto it = keys.begin();
        auto itNext = std::next(it);
        for (; itNext != keys.end(); ++it, ++itNext)
        {
            if (it->time <= _t && _t <= itNext->time)
            {
                float t = (itNext->time - _t) / (itNext->time - it->time);
                if (itNext->pEasingFunc)
                    t = itNext->pEasingFunc(t);
                else if (it->pEasingFunc)
                    t = it->pEasingFunc(t);
                return Lerp(it->value, itNext->value, t);
            }
        }
        return keys.back().value;
    }
};

struct ParticleInitParam
{
    // 有効時間
    float lifeTime = 1.0f;
    bool isInfiniteLife = false;

    // サイズ
    Vector3 size = { 1,1,1 };
    ValueTransition<Vector3> sizeTransition;

    // 回転
    Vector3 rotate = { 0,0,0 };
    ValueTransition<Vector3> rotateTransition;

    // 位置
    Vector3 position = { 0,0,0 };

    // スピード
    float speed = 0.0f;
    ValueTransition<float> speedTransition;

    // 方向
    Vector3 direction = { 0,0,0 };

    // 加速度，重力
    Vector3 acceleration = { 0,0,0 };

    // 減速係数
    float deceleration = 0.0f;

    Vector4 color = { 1,1,1,1 };
    // 色 RGB
    ValueTransition<Vector3> colorTransition;
    // 色 A
    ValueTransition<float> alphaTransition;

    Matrix4x4 directionMatrix = Matrix4x4::Identity();

    //TODO: UVTransform
    // 上下左右に移動
    // 分割してアニメーション あるいはランダム
};

