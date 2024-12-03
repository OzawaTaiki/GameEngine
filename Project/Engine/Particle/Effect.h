#pragma once

#include "Particle/ParticleEmitters.h"

#include <string>
#include <list>
#include <memory>

class Effect
{
private:
    struct EmitterData
    {
        std::string         name;                       // エミッターの名前
        std::unique_ptr<ParticleEmitter> pEmitterPtr;   // エミッターのポインタ
        float               delayTime;                  // 発生までの遅延時間
        float               duration;                   // エミッターの持続時間
        bool                loop;                       // ループするか
    };

public:
    Effect() = default;
    ~Effect() = default;

    void Initialize(const std::string& _name);
    void Update();

    ParticleEmitter* AddEmitter(const std::string& _name, float _delayTime = 0, float _duration = 1, bool _loop = false);

    std::list<ParticleEmitter*> GetEmitters() const;
    std::string GetName() const { return name_; }

    void SetActive(bool _active) { isActive_ = _active; }

    void Save();
private:

    std::string                 name_;              // エフェクトの名前
    std::list<EmitterData>      emitters_;          // エミッターのリスト


    float                       elapsedTime_;       // 経過時間
    bool                        isLoop_;            // ループするか
    bool                        isActive_;          // アクティブか
};
