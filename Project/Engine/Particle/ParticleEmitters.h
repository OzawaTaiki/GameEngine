#pragma once

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <string>


template<class T>
struct Range
{
    T min;
    T max;
};
// パーティクル生成時の初期設定
struct EmitParticleSettings
{
    Range<float>            lifeTime = {1,1};                   // 有効時間

    Range<Vector3>          size = { {1,1,1},{1,1,1} };         // サイズ
    Range<Vector3>          rotate = { {},{} };                 // 回転

    Range<float>            spped = {0,1};                      // スピード
    Range<Vector3>          direction = { {},{} };              // 方向
    Range<Vector3>          acceleration = { {},{} };           // 加速度，重力

    Range<Vector4>          color = { {1,1,1,1},{1,1,1,1} };    // 色

};
enum class EmitterShape
{
    Box,
    Shpere,
    Circle,

    None
};

enum class ParticleDirection
{
    Inward,
    Outward,
    Random
};

/*

    struct colorKey{
        float time;
        vector4 color;
    }
    std::vector<colorKey> colorKeys;

    １グループ内での発生タイミング ０～



*/


class Particle;
class ParticleEmitter
{
public:

    ParticleEmitter() = default;
    ~ParticleEmitter() = default;


    void Setting(const std::string& _name);

    void Update();
    void Draw();

    void SetShape_Box(const Vector3& _size);
    void SetShape_Sphere(float _radius ) ;
    void SetShape_Circle(float _radius);

    void SetWorldMatrix(const Matrix4x4* _mat) { parentMatWorld_ = _mat; }
    void SetModelPath(const std::string& _path) { useModelPath_ = _path; }
    void SetCenter(const Vector3& _center) { position_ = _center; }
    void SetActive(bool _active) { isActive_ = _active; }

    bool IsActive() const { return isActive_; }
    bool EnableBillboard() const { return isEnableBillboard_; }
    std::string GetName() const { return name_; }
    std::string GetModelPath() const { return useModelPath_; }

    void ShowDebugWinsow();

    EmitParticleSettings    setting_{};
private:

    std::string             name_;
    std::string             useModelPath_;
    char                    name_buffer_[256];                 //imgui で入力用
    std::string             useTextruePath_;
    char                    texture_buffer_[256];              //imgui で入力用

    float                   currentTime_            = 0;
    float                   deltaTime_              = 1.0f / 60.0f;
    float                   emitTime_               = 0;

    EmitterShape            shape_ = EmitterShape::None;
    ParticleDirection       particleDirection_ = ParticleDirection::Random;
    const Matrix4x4*        parentMatWorld_ = nullptr;
    Vector3                 position_;
    Vector3                 offset_;
    Vector3                 rotate_;
    Vector3                 size_;
    float                   radius_;

    bool                    loop_;              // ループするか
    bool                    fadeAlpha_;         // 生成後にアルファを変更するか
    bool                    changeSize_;        // 生成後にサイズを変更するか
    bool                    changeColor_;       // 生成後に色を変更するか
    bool                    randomColor_;       // 色をランダムで生成するか
    bool                    isEnableBillboard_; // ビルボードを使用するか
    float                   delayTime_;         // 発生までの遅延時間
    float                   fadeStartRatio_;    // アルファを変え始める割合
    uint32_t                maxParticles_;      // 最大数
    uint32_t                countPerEmit_;      // 回当たりの発生数
    uint32_t                emitPerSec_;        // 秒あたりの発生回数
    uint32_t                emitRepeatCount_;   // 繰り返し回数
    uint32_t                emitCount_;         // 発生回数

    bool                    isActive_ = false;

    Particle GenerateParticleData();
};