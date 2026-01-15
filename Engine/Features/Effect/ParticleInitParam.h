#pragma once
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Math/MyLib.h>
#include <Math/Easing.h>

#include <list>
#include <functional>
#include <array>


namespace Engine {

struct ParticleInitParam
{
    // 有効時間
    float lifeTime = 1.0f;
    bool isInfiniteLife = false;

    // サイズ
    Vector3 size = { 1,1,1 };
    //ValueTransition<Vector3> sizeTransition;

    // 回転
    Vector3 rotate = { 0,0,0 };
    //ValueTransition<Vector3> rotateTransition;
    Vector3 rotationSpeed = { 0,0,0 }; // 回転速度

    // 位置
    Vector3 position = { 0,0,0 };

    // スピード
    float speed = 0.0f;
    //ValueTransition<float> speedTransition;

    // 方向
    Vector3 direction = { 0,0,0 };

    // 加速度，重力
    Vector3 acceleration = { 0,0,0 };

    Vector4 color = { 1,1,1,1 };

    std::array<bool, 3> isBillboard = { false,false,false }; // ビルボード xyz

    //TODO: UVTransform
    // 上下左右に移動
    // 分割してアニメーション あるいはランダム
};

} // namespace Engine
