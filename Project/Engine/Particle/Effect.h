#pragma once

#include "Particle/ParticleEmitters.h"

#include <string>
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

    void Save();
private:

    std::string                     name_;              // エフェクトの名前
    std::list<ParticleEmitter>      emitters_;          // エミッターのリスト


    float                       elapsedTime_;       // 経過時間
    bool                        isLoop_;            // ループするか
    bool                        isActive_;          // アクティブか
};
