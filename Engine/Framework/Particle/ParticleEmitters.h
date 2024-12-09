#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Physics/Math/Matrix4x4.h>

#include <Systems/Config/Config.h>
#include <Framework/Particle/ParticleInitParam.h>

#include <string>
#include <array>
#include <vector>

template<class T>
struct Range
{
    T min;
    T max;
};
template<typename T>
struct SettingGroup
{
    bool random;
    bool fixed;

    Range<T> value;

};

// パーティクル生成時の初期設定
struct EmitParticleSettings
{
    SettingGroup<float>     lifeTime = { false,true,{1,1} };
     SettingGroup<Vector3>   size = { false,true,{{1,1,1},{1,1,1}} };
     SettingGroup<Vector3>   rotate = { false,true,{{1,1,1},{1,1,1}} };
     SettingGroup<float>     speed = { false,true,{0,1} };
     SettingGroup<Vector3>   direction = { false,true,{{-1,-1,-1},{1,1,1}} };
     SettingGroup<Vector3>   acceleration = { false,true,{{0,0,0},{0,0,0}} };
     SettingGroup<Vector4>   color = { false,true,{} };
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
    void Draw() const;

    void SetShape_Box(const Vector3& _size);
    void SetShape_Sphere(float _radius ) ;
    void SetShape_Circle(float _radius);

    void SetWorldMatrix(const Matrix4x4* _mat) { parentMatWorld_ = _mat; }
    void SetModelPath(const std::string& _path) { useModelPath_ = _path; }
    void SetCenter(const Vector3& _center) { position_ = _center; }
    void SetActive(bool _active);
    void SetAlive(bool _alive) { isAlive_ = _alive; }

    bool IsActive() const { return isActive_; }
    bool IsAlive() const { return isAlive_; }
    bool EnableBillboard() const { return isEnableBillboard_; }
    bool ShouldFaceDirection() const { return isLengthScalingEnabled_; }

    float GetDelayTime() const { return delayTime_; }
    float GetDuration() const { return duration_; }
    std::array<bool, 3> GetBillboardAxes() const { return billboardAxes_; }

    std::string GetName() const { return name_; }
    std::string GetModelPath() const { return useModelPath_; }


    void Save()const;
    bool ShowDebugWinsow();

    void Reset();

private:
    EmitParticleSettings    setting_{};



    ParticleInitParam       parametor_ = {}; // NEW!!!

    std::string             name_;
    std::string             useModelPath_;
    char                    name_buffer_[256];                 //imgui で入力用
    std::string             useTextruePath_;
    char                    texture_buffer_[256];              //imgui で入力用

    float                   currentTime_            = 0;
    float                   deltaTime_              = 1.0f / 60.0f;
    float                   emitTime_               = 0;

    KeyFrame<Vector3>       addSize_ = {};
    KeyFrame<Vector3>       addRotate_ = {};
    KeyFrame<float>         addSpeed_ = {};
    KeyFrame<Vector3>       addColor_ = {};
    KeyFrame<float>         addAlpha_ = {};

    EmitterShape            shape_ = EmitterShape::None;
    ParticleDirection       particleDirection_ = ParticleDirection::Random;
    const Matrix4x4*        parentMatWorld_ = nullptr;
    Vector3                 position_;
    Vector3                 offset_;
    Vector3                 rotate_;
    Vector3                 size_ = { 1,1,1 };
    float                   radius_ = 1;

    bool                    loop_;                          // ループするか
    bool                    fadeAlpha_;                     // 生成後にアルファを変更するか
    bool                    changeSize_;                    // 生成後にサイズを変更するか
    bool                    changeColor_;                   // 生成後に色を変更するか
    bool                    randomColor_;                   // 色をランダムで生成するか
    bool                    isEnableBillboard_;             // ビルボードを使用するか
    bool                    isLengthScalingEnabled_;        // 方向を向くか
    bool                    isInfiniteLife_;                // 無限寿命か

    std::array <bool, 3>    lockRotationAxes_;              // 軸を固定するか
    Vector3                 lockRotationAxesValue_;         // 固定した軸の値

    std::array <bool, 3>     billboardAxes_ = { 0 };        // ビルボードの軸

    bool                   isFixedDirection_    = false;    // 方向を固定するか
    bool                   isFixedSpeed_        = false;    // スピードを固定するか
    bool                   isFixedSize_         = false;    // サイズを固定するか
    bool                   isFixedLifeTime_     = false;    // 寿命を固定するか
    bool                   isFixedColor_        = false;    // 色を固定するか
    bool                   isFixedAcceleration_ = false;    // 加速度を固定するか

    float                   decelelation_       = 0;        // 減速係数

    float                   delayTime_;                     // 発生までの遅延時間
    float                   fadeStartRatio_;                // アルファを変え始める割合
    float                   duration_ = 1;                  // エミッターの持続時間
    uint32_t                maxParticles_;                  // 最大数
    uint32_t                countPerEmit_ = 1;              // 回当たりの発生数
    uint32_t                emitPerSec_ = 1;                // 秒あたりの発生回数
    uint32_t                emitRepeatCount_ = 1;           // 繰り返し回数
    uint32_t                emitCount_ = 1;                 // 発生回数

    bool                    isActive_ = false;              // アクティブか
    bool                    isAlive_ = true;                // まだ生きているか


    Particle GenerateParticleData();

    void DisplayFlags();

    void DisPlayEmitterParameters();

    void DisplayLifeTimeParameters();
    void DisplaySizeParameters();
    void DisplaySpeedParameters();
    void DisplayDirectionParameters();
    void DisplayAccelerationParameters();
    void DisplayColorParameters();

};
