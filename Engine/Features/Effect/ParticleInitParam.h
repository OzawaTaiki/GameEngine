#pragma once
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Math/MyLib.h>
#include <Math/Easing.h>
#include <Features/Json/Loader/JsonLoader.h>
#include <list>
#include <functional>

template <typename T>
struct TransitionKeyFrame
{
    // time で value に変化する
    float time = 0.99f;
    T value = {};

    // イージング関数 indexで持つ
    int32_t easingFuncNum = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TransitionKeyFrame, time, value, easingFuncNum)
};

template <typename T>
struct ValueTransition
{
    // 変更するかのフラグ
    bool isChange = false;

    // 変更する値
    std::list<TransitionKeyFrame<T>> keys = {};



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
                float t = (_t) / (itNext->time - it->time);

                t = Easing::SelectFuncPtr(itNext->easingFuncNum)(t);

                return Lerp(it->value, itNext->value, t);
            }
        }
        return keys.back().value;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ValueTransition, isChange, keys)
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

