#pragma once

#include <Framework/Particle/ParticleEmitters.h>
#include <Systems/Time/GameTime.h>
#include <Systems/JsonBinder/JsonBinder.h>
#include <string>
#include <vector>
#include <list>
#include <memory>

class Effect
{
public:
    Effect() = default;
    ~Effect();

    void Initialize(const std::string& _name);
    void Update();

    void AddEmitter(const std::string& _name);

    std::list<ParticleEmitter*> GetEmitters() const;
    std::string GetName() const { return name_; }

    void SetActive(bool _active);

    void ExclusionEmitter(const std::string& _name);

    void DebugShowForEmitterCreate();

    void SetTimeChannel(const std::string& _channel);

    void Save()const;
private:
    void Reset();

    static const uint32_t          kMaxEmitters = 20;  // 最大エミッター数

    std::string                     name_;              // エフェクトの名前
    std::list<std::unique_ptr<ParticleEmitter>>      emitters_;          // エミッターのリスト
    std::vector<std::string>        emitterNames_;      // エミッターの名前リスト

    std::string                     addEmitterName_;    // 追加するエミッターの名前
    char                            emitterBuf_[256];    // エミッターの名前バッファ


    float                           elapsedTime_;       // 経過時間
    bool                            isLoop_;            // ループするか
    bool                            isActive_;          // アクティブか

    std::string                     timeChannel_    = "default";
    GameTime* gameTime_ = nullptr;

    std::unique_ptr<JsonBinder>     jsonBinder_ = nullptr;  // 設定ファイル
    //std::unique_ptr<Config> config_ = nullptr;
};
