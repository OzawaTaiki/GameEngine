#pragma once

#include "Particle/ParticleEmitters.h"
#include <Utility/Config.h>
#include <string>
#include <vector>
#include <list>
#include <memory>

class Effect
{
private:

public:
    Effect() = default;
    ~Effect() = default;

    void Initialize(const std::string& _name);
    void Update();

    ParticleEmitter* AddEmitter(const std::string& _name);

    std::list<ParticleEmitter*> GetEmitters() const;
    std::string GetName() const { return name_; }

    void SetActive(bool _active) { isActive_ = _active; }

    void ExclusionEmitter(const std::string& _name);

    void Save()const;
private:

    static const uint32_t          kMaxEmitters = 20;  // 最大エミッター数

    std::string                     name_;              // エフェクトの名前
    std::list<ParticleEmitter>      emitters_;          // エミッターのリスト
    std::vector<std::string>        emitterNames_;      // エミッターの名前リスト


    float                           elapsedTime_;       // 経過時間
    bool                            isLoop_;            // ループするか
    bool                            isActive_;          // アクティブか

    std::unique_ptr<Config> config_ = nullptr;  // 設定ファイル
};
