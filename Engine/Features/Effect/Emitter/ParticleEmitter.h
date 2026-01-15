#pragma once

#include <Math/Random/RandomGenerator.h>
#include <Features/Effect/ParticleInitParam.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Json/JsonBinder.h>

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <cstdint>
#include <functional>
#include <vector>

// Range構造体 - テンプレート化

namespace Engine {

template<typename T>
struct Range {
    bool isRandom = false;
    T value;
    T minV;
    T maxV;

    Range(T _defaultValue, T _minValue, T _maxValue)
        : value(_defaultValue), minV(_minValue), maxV(_maxValue) {
    }

    T GetValue() const {
        if (isRandom) {
            return RandomGenerator::GetInstance()->GetRandValue(minV, maxV);
        }
        return value;
    }

    void SetValue(T _newValue) { value = _newValue; }
    void SetRange(T _minValue, T _maxValue) {
        minV = _minValue;
        maxV = _maxValue;
        isRandom = true;
    }
    void SetFixed(T _fixedValue) {
        value = _fixedValue;
        isRandom = false;
    }
};

// Enum定義
enum class EmitterShape : uint32_t {
    Box = 0,
    Sphere = 1,
    Count
};

enum class ParticleDirectionType : uint32_t {
    Outward = 0,
    Inward = 1,
    Random = 2,
    Fixed = 3,
    Count
};

enum class ParticleLifeTimeType : uint32_t {
    Infinite = 0,
    Random = 1,
    Fixed = 2,
    Count
};

// パーティクル初期化パラメータ
struct ParticleInitParamsForGenerator
{
    // 位置関連
    Range<Vector3> boxInnerSize = { Vector3(0,0,0), Vector3(0,0,0), Vector3(0,0,0) };
    Range<float> sphereOffset = { 0.0f, 0.0f, 0.0f };

    // 動作関連
    Range<Vector3> direction = { Vector3(0,0,0), Vector3(-1,-1,-1), Vector3(1,1,1) };
    ParticleDirectionType directionType = ParticleDirectionType::Outward;
    Range<float> speed = { 1.0f, 0.5f, 2.0f };
    Range<float> deceleration = { 0.0f, 0.0f, 1.0f };

    // 寿命関連
    Range<float> lifeTime = { 1.0f, 0.5f, 2.0f };
    ParticleLifeTimeType lifeTimeType = ParticleLifeTimeType::Fixed;

    // 見た目関連
    Range<Vector3> size = { Vector3(1,1,1), Vector3(0.5f,0.5f,0.5f), Vector3(2,2,2) };
    Range<Vector3> rotation = { Vector3(0,0,0), Vector3(0,0,0), Vector3(6.28318f,6.28318f,6.28318f) };
    Range<Vector3> rotationSpeed = { Vector3(0,0,0), Vector3(-3.14159f,-3.14159f,-3.14159f), Vector3(3.14159f,3.14159f,3.14159f) };
    Range<Vector3> colorRGB = { Vector3(1,1,1), Vector3(0,0,0), Vector3(1,1,1) };
    Range<float> colorA = { 1.0f, 0.0f, 1.0f };

    std::array<bool, 3> billboard = { false, false, false };
    std::string textureName = "uvChecker.png";
    std::vector<std::string> modifiers;
};

class ParticleEmitter
{
public:
    ParticleEmitter() = default;
    ~ParticleEmitter() = default;

    // === 初期化・基本操作 ===
    bool Initialize(const std::string& _name);  // 保存するため const std::string&
    void Update(float _deltaTime);
    void Reset();

    // === パーティクル生成 ===
    void GenerateParticles();
    void EmitSingle();
    void EmitBurst(uint32_t _count);

    // === 状態管理 ===
    void SetActive(bool _active) { isActive_ = _active; }
    bool IsActive() const { return isActive_; }
    void SetAlive(bool _alive) { isAlive_ = _alive; }
    bool IsAlive() const { return isAlive_; }

    // === 設定系 (保存するため const std::string&) ===
    void SetName(const std::string& _name);
    void SetModelName(const std::string& _modelName);
    void SetTextureName(const std::string& _textureName);
    void SetTimeChannel(const std::string& _channel);

    // === 検索・比較系 (一時的なため std::string_view) ===
    bool HasModifier(std::string_view _modifierName) const;

    // === ファイル操作系 (一時的なため std::string_view) ===
    bool SaveToFile() const;
    void LoadTexture(std::string_view _texturePath);
    void LoadModel(std::string_view _modelPath);

    // === 取得系 ===
    const std::string& GetName() const { return name_; }
    const std::string& GetModelName() const { return useModelName_; }
    const std::string& GetTextureName() const { return initParams_.textureName; }
    float GetDelayTime() const { return delayTime_; }
    float GetDuration() const { return lifeTime; }
    float GetElapsedTime() const { return elapsedTime_; }

    // === 変形・位置系 ===
    void SetParentTransform(WorldTransform* _parentTransform) { parentTransform_ = _parentTransform; }
    void SetOffset(const Vector3& _offset) { offset_ = _offset; }
    void SetPosition(const Vector3& _position) { position_ = _position; }
    void SetRotation(const Quaternion& _rotation) { rotation_ = _rotation; }
    void SetScale(const Vector3& _scale) { scale_ = _scale; }

    const Vector3& GetPosition() const { return position_; }
    const Quaternion& GetRotation() const { return rotation_; }
    const Vector3& GetScale() const { return scale_; }

    // === モディファイア管理 ===
    void AddModifier(const std::string& _modifierName);  // 保存
    bool RemoveModifier(std::string_view _modifierName); // 検索
    void ClearModifiers() { initParams_.modifiers.clear(); }
    const std::vector<std::string>& GetModifiers() const { return initParams_.modifiers; }

    // === デバッグ・エラー ===
    std::string GetLastError() const { return lastError_; }

#ifdef _DEBUG
    void ShowDebugWindow();
#endif

private:
    // === コア設定 ===
    std::string name_ = "Emitter";
    bool isActive_ = false;
    bool isAlive_ = true;

    // === 親子関係 ===
    WorldTransform* parentTransform_ = nullptr;
    Vector3 offset_ = { 0, 0, 0 };

    // === 変形 ===
    Vector3 position_ = { 0, 0, 0 };
    Vector3 rotationEuler_ = { 0, 0, 0 }; // Euler角
    Quaternion rotation_ = { 0, 0, 0, 1 };
    Vector3 scale_ = { 1, 1, 1 };

    // === レンダリング設定 ===
    PSOFlags::BlendMode blendMode_ = PSOFlags::BlendMode::Add;
    bool cullBack_ = false;

    // === エミッター形状 ===
    EmitterShape shape_ = EmitterShape::Box;
    Vector3 boxSize_ = { 1, 1, 1 };
    float sphereRadius_ = 1.0f;

    // === 発生設定 ===
    uint32_t emitPerSecond_ = 10;
    uint32_t emitCount_ = 1;
    std::string useModelName_ = "cube/cube.obj";

    // === タイミング ===
    bool isLoop_ = false;
    float elapsedTime_ = 0.0f;
    float delayTime_ = 0.0f;
    float lifeTime = 0.0f;
    std::string timeChannel_ = "default";

    // === パーティクル初期値 ===
    ParticleInitParamsForGenerator initParams_;

    // === データ管理 ===
    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;

    // === 統計・デバッグ ===
    mutable std::string lastError_;

#ifdef _DEBUG
    // === ImGui用バッファ ===
    static int s_nextID_;
    int instanceID_ = 0;
    char nameBuf_[256] = {};
    char modelPath_[256] = {};
    char modelName_[256] = {};
    char texturePath_[256] = {};
    char textureRoot_[256] = "Resources/images/";
    char modifierName_[256] = {};
#endif

    // === 内部ヘルパー ===
    void InitJsonBinder();
    void SetError(std::string_view _error) const { lastError_ = _error; }
    void ClearError() const { lastError_.clear(); }
    bool ValidateSettings() const;

#ifdef _DEBUG
    // === ImGuiヘルパー ===
    void DebugWindowForSize();
    void DebugWindowForPosition();
    void DebugWindowForDirection();
    void DebugWindowForSpeed();
    void DebugWindowForRotation();
    void DebugWindowForDeceleration();
    void DebugWindowForLifeTime();
    void DebugWindowForColor();
    void DebugWindowForModifier();
#endif
};

} // namespace Engine
