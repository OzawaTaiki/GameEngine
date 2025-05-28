#pragma once
#include <Math/Random/RandomGenerator.h>
#include <Features/Effect/ParticleInitParam.h>
#include <Core/BlendMode.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Json/JsonBinder.h>

#include <string>
#include <array>
#include <memory>
#include <cstdint>


template<typename T>
struct Range {
    bool isRandom = false;  // ランダム値かどうか
    T value;                // 固定値（isRandom = falseの場合）
    T min;                  // 最小値（isRandom = trueの場合）
    T max;                  // 最大値（isRandom = trueの場合）

    // コンストラクタ
    Range(T defaultValue, T minValue, T maxValue)
        : value(defaultValue){
        min = minValue;
        max = maxValue;
    }


    // 現在の値を取得（固定かランダムに応じて）
    T GetValue() const {
        if (isRandom) {
            // ランダム値を生成
            return RandomGenerator::GetInstance()->GetRandValue(min, max);
        }
        return value;  // 固定値を返す
    }
};

enum class EmitterShape
{
    Box,    // 立方体
    Sphere, // 球形
    //Cone,   // 円錐
    //Cylinder, // 円柱
    //Plane,  // 平面
};

enum class ParticleDirectionType
{
    Outward,   // 外側に向かう
    Inward,    // 内側に向かう
    Random,    // ランダム
    Fixed      // 固定

};

enum class ParticleLifeTimeType
{
    Infinite,  // 無限
    Random,    // ランダム
    Fixed      // 固定
};


struct ParticleInitParamsForGenerator
{
    // パーティクルの初期位置
    // Boxの場合は、Boxの中心からのオフセット
    Range<Vector3> boxOffset = { 0.0f, 0.0f, 0.0f };
    // Sphereの場合は、Sphereの中心からのオフセット
    Range<float> sphereOffset = { 0.0f, 0.0f, 0.0f };


    // パーティクルの初期速度
    Range<Vector3> direction = { Vector3(0, 0, 0), Vector3(-1, -1, -1), Vector3(1, 1, 1) };
    ParticleDirectionType directionType = ParticleDirectionType::Outward; // 方向のタイプ
    // パーティクルの速さ
    Range<float> speed = { 1.0f, 0.5f, 2.0f };
    // パーティクルの寿命
    Range<float> lifeTime = { 1.0f, 0.5f, 2.0f };
    ParticleLifeTimeType lifeTimeType = ParticleLifeTimeType::Fixed; // 寿命のタイプ

    // パーティクルの減速率
    Range<float> deceleration = { 0.0f, 0.0f, 1.0f };


    // 回転パラメータを追加
    Range<Vector3> rotation = { Vector3(0, 0, 0), Vector3(0,0,0), Vector3(6.28318f ,6.28318f ,6.28318f)}; // 0〜2π（360度)
    Range<Vector3> rotationSpeed = { Vector3(0, 0, 0), Vector3(-3.14159f,-3.14159f,-3.14159f), Vector3(3.14159f,3.14159f,3.14159f) }; // -π〜π（-180度〜180度/秒）


    // パーティクルの初期色
    Range<Vector3> colorRGB = { Vector3(1, 1, 1), Vector3(0, 0, 0), Vector3(1, 1, 1) };
    Range<float> colorA = { 1.0f, 0.0f, 1.0f }; // アルファ値の範囲
    // パーティクルの初期サイズ
    Range<Vector3> size = { Vector3(1.0f,1.0f,1.0f), Vector3(0.5f,0.5f,0.5f), Vector3(2.0f,2.0f,2.0f) };

    std::array<bool, 3> billboard = { false, false, false }; // ビルボードのタイプ

    std::string textureName = "uvChecker.png"; // テクスチャの名前（デフォルト

    std::vector<std::string> modifiers; // モディファイアのリスト

};

class ParticleEmitter
{
public:
    ParticleEmitter() {};
    ~ParticleEmitter() {};

    void Initialize(const std::string& _name);
    void Update(float _deltaTime);

    void ShowDebugWindow();


    void GenerateParticles();

private:

    void InitJsonBinder();

    // エミッターの名前
    std::string name_ = "Emitter"; // エミッターの名前

    // 有効フラグ
    bool isActive_ = false; // アクティブか

    // 親のWorldTransform
    WorldTransform* parentTransform_ = nullptr; // 親のワールドトランスフォーム
    // 親があるときのオフセット
    Vector3 offset_ = { 0, 0, 0 }; // 親からのオフセット

    BlendMode blendMode_ = BlendMode::Add; // ブレンドモード
    bool cullBack_ = false; // バックカリングするか

    // エミッターの設定
    EmitterShape shape_ = EmitterShape::Box; // エミッターの形状

    uint32_t emitPerSecond_ = 10; // 秒間の発生数
    // 一度に発生するパーティクルの数
    uint32_t emitCount_ = 1; // 一度に発生するパーティクルの数

    std::string useModelName_ = "plane/plane.gltf"; // 使用するモデルの名前

#ifdef _DEBUG
    char modelPath_[256];
    char modelName_[256];
    char nameBuf_[256];
    char texturePath_[256];
    char textureRoot_[256] = "Resources/images/"; // テクスチャのルートパス
    char modifierName[256]; // モディファイアの名前
#endif _DEBUG



    bool isLoop_ = false; // ループするか
    float elapsedTime_ = 0.0f; // 経過時間
    float delayTime_ = 0.0f; // 遅延時間
    float lifeTime = 0.0f; // 寿命

    Vector3 position_ = { 0, 0, 0 }; // エミッターの位置
    Quaternion rotation_ = { 0, 0, 0, 1 }; // エミッターの回転


    //---------------
    // shapeごとのパラメータ

    // Box
    Vector3 boxSize_ = { 1, 1, 1 }; // ボックスのサイズ

    // Sphere
    float sphereRadius_ = 1.0f; // 球の半径


    // パーティクルの初期値用
    ParticleInitParamsForGenerator initParams_; // パーティクルの初期値設定用

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr; // JsonBinder



    //各パラメータごとの設定用imgui関数
#ifdef _DEBUG

    void DebugWindowForSize();
    void DebugWindowForPosition();
    void DebugWindowForDirection();
    void DebugWindowForSpeed();
    void DebugWindowForRotation();
    void DebugWindowForDeceleration();
    void DebugWindowForLifeTime();
    void DebugWindowForColor();

    void DebugWindowForModifier();

#endif // _DEBUG



};