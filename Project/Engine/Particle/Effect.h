#pragma once

#include "Particle/ParticleEmitters.h"

#include <string>
#include <list>

class Effect
{
private:
    struct EmitterData
    {
        std::string         name;               // エミッターの名前
        ParticleEmitter*    pEmitterPtr;        // エミッターのポインタ
        float               delayTime;          // 発生までの遅延時間
        float               duration;           // エミッターの持続時間
        bool                loop;               // ループするか
    };

public:
    Effect() = default;
    ~Effect() = default;

    void Update();

    void AddEmitter(const std::string& _name, ParticleEmitter* _pEmitter, float _delayTime, float _duration, bool _loop);


    void Save();
private:

    std::list<EmitterData>      emitters_;          // エミッターのリスト

    
    float                       elapsedTime_;       // 経過時間
    bool                        isLoop_;            // ループするか
    bool                        isActive_;          // アクティブか
};
