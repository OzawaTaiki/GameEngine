#pragma once

#include <System/Time/GameTime.h>
#include <Features/Json/JsonBinder.h>
#include <Features/Effect/Emitter/ParticleEmitter.h>
#include <Math/MyLib.h>

#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <unordered_map>


namespace Engine {

class Effect
{
public:
    // === ファクトリーメソッド ===
    static std::unique_ptr<Effect> Create(const std::string& _name);  // 保存
    static std::unique_ptr<Effect> CreateFromFile(std::string_view _filePath);  // ファイル名

    Effect() = default;
    ~Effect();

    // === 初期化 ===
    bool Initialize(const std::string& _name);  // 保存

    // === 再生制御 ===
    void Play();
    void Stop();
    void Pause();
    void Resume();
    void Reset();
    void Restart() { Reset(); Play(); }

    // === 状態確認 ===
    bool IsPlaying() const { return isActive_ && !isPaused_; }
    bool IsPaused() const { return isPaused_; }
    bool IsActive() const { return isActive_; }
    bool IsLoop() const { return isLoop_; }
    bool IsComplete() const;

    // === 更新・描画 ===
    void Update();
    void Draw();
    void DrawShadow();

    // === エミッター管理 (保存系) ===
    bool AddEmitter(const std::string& _name);
    bool CreateEmitter(const std::string& _name, const std::string& _templateName = "");

    // === エミッター管理 (検索系) ===
    bool RemoveEmitter(std::string_view _name);
    ParticleEmitter* FindEmitter(std::string_view _name) const;
    bool HasEmitter(std::string_view _name) const;
    void ClearEmitters();

    // === エミッター取得 ===
    std::list<ParticleEmitter*> GetEmitters() const;
    std::vector<std::string> GetEmitterNames() const;
    size_t GetEmitterCount() const { return emitters_.size(); }
    ParticleEmitter* GetEmitterAt(size_t _index) const;

    // === 設定 (保存系) ===
    void SetName(const std::string& _name);
    void SetTimeChannel(const std::string& _channel);
    void SetLoop(bool _loop) { isLoop_ = _loop; }

    // === 変形・位置 ===
    void SetParentMatrix(const Matrix4x4* _parentMat);
    void SetPosition(const Vector3& _position);
    void SetRotation(const Quaternion& _rotation);
    void SetScale(const Vector3& _scale);

    const Vector3& GetPosition() const { return position_; }
    const Quaternion& GetRotation() const { return rotation_; }
    const Vector3& GetScale() const { return scale_; }

    // === タイミング制御 ===
    void SetPlaybackSpeed(float _speed) { playbackSpeed_ = (std::max)(0.0f, _speed); }
    float GetPlaybackSpeed() const { return playbackSpeed_; }
    void SetStartDelay(float _delay) { startDelay_ = (std::max)(0.0f, _delay); }
    float GetStartDelay() const { return startDelay_; }

    // === 情報取得 ===
    const std::string& GetName() const { return name_; }
    float GetElapsedTime() const { return elapsedTime_; }
    float GetDuration() const;
    float GetProgress() const;  // 0.0f〜1.0f

    // === ファイル操作 (一時的) ===
    bool LoadFromFile(std::string_view _filePath);
    bool SaveToFile() const;
    bool Save() const;

    // === イベントコールバック ===
    using EffectCallback = std::function<void(Effect*)>;
    void SetOnPlayCallback(EffectCallback _callback) { onPlayCallback_ = _callback; }
    void SetOnStopCallback(EffectCallback _callback) { onStopCallback_ = _callback; }
    void SetOnCompleteCallback(EffectCallback _callback) { onCompleteCallback_ = _callback; }
    void SetOnLoopCallback(EffectCallback _callback) { onLoopCallback_ = _callback; }

    // === バッチ操作 ===
    void SetAllEmittersActive(bool _active);
    void SetEmitterGroupActive(std::string_view _groupName, bool _active);
    void ScaleAllEmitters(float _scale);

    // === パフォーマンス統計 ===
    struct Statistics {
        uint32_t totalParticles = 0;
        uint32_t activeParticles = 0;
        float averageUpdateTime = 0.0f;
        float averageDrawTime = 0.0f;
        uint32_t drawCalls = 0;
    };
    const Statistics& GetStatistics() const { return statistics_; }
    void ResetStatistics() { statistics_ = {}; }

    // === レイヤー管理 ===
    void SetRenderLayer(uint32_t _layer) { renderLayer_ = _layer; }
    uint32_t GetRenderLayer() const { return renderLayer_; }
    void SetVisible(bool _visible) { isVisible_ = _visible; }
    bool IsVisible() const { return isVisible_; }

    // === エラー管理 ===
    std::string GetLastError() const { return lastError_; }
    bool HasError() const { return !lastError_.empty(); }
    void ClearError() const { lastError_.clear(); }

#ifdef _DEBUG
    // === デバッグ表示 ===
    void ShowDebugWindow();
    void ShowEmitterList();
    void ShowStatistics();
    bool EnableDebugDraw() const { return enableDebugDraw_; }
    void SetEnableDebugDraw(bool _enable) { enableDebugDraw_ = _enable; }
#endif

private:
    static const uint32_t kMaxEmitters = 50;

    // === 基本情報 ===
    std::string name_;
    bool isActive_ = false;
    bool isPaused_ = false;
    bool isLoop_ = false;
    bool isVisible_ = true;

    // === エミッター管理 ===
    std::list<std::unique_ptr<ParticleEmitter>> emitters_;
    std::vector<std::string> emitterNames_;
    std::unordered_map<std::string, size_t> emitterIndexMap_;  // 高速検索用

    // === タイミング ===
    float elapsedTime_ = 0.0f;
    float playbackSpeed_ = 1.0f;
    float startDelay_ = 0.0f;
    std::string timeChannel_ = "default";
    GameTime* gameTime_ = nullptr;

    // === 変形 ===
    Vector3 position_ = { 0, 0, 0 };
    Quaternion rotation_ = { 0, 0, 0, 1 };
    Vector3 scale_ = { 1, 1, 1 };
    Matrix4x4 worldMatrix_ = Matrix4x4::Identity();

    // === レンダリング ===
    uint32_t renderLayer_ = 0;

    // === データ管理 ===
    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;

    // === コールバック ===
    EffectCallback onPlayCallback_;
    EffectCallback onStopCallback_;
    EffectCallback onCompleteCallback_;
    EffectCallback onLoopCallback_;

    // === 統計・デバッグ ===
    Statistics statistics_;
    mutable std::string lastError_;

#ifdef _DEBUG
    bool enableDebugDraw_ = false;
    char newEmitterNameBuf_[256] = "NewEmitter";
    char presetNameBuf_[256] = {};
#endif

    // === 内部ヘルパー ===
    void UpdateWorldMatrix();
    void UpdateEmitters();
    void SetError(std::string_view _error) const { lastError_ = _error; }
    bool ValidateEmitterName(std::string_view _name) const;
    void RebuildIndexMap();
    void InvokeCallback(EffectCallback& _callback);

#ifdef _DEBUG
    void DebugDrawBounds();
    void DebugDrawEmitters();
#endif
};

} // namespace Engine
