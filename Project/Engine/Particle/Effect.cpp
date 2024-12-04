#include "Effect.h"
#include "Utility/Time.h"
#include "Utility/ConfigManager.h"


void Effect::Initialize(const std::string& _name)
{
    name_ = _name;
}

void Effect::Update()
{
    if (!isActive_)
    {
        elapsedTime_ = 0;
        return;
    }

    elapsedTime_ += Time::GetDeltaTime<float>();

    for (auto& emitter : emitters_)
    {
        // 遅延時間が経過していなかったらスキップ
        if (elapsedTime_<=emitter.delayTime)
        {
            emitter.pEmitterPtr->SetActive(false);
            continue;
        }

        // エミッターの持続時間が経過していたらスキップ
        if (emitter.duration <= elapsedTime_ - emitter.delayTime)
        {
            emitter.pEmitterPtr->SetActive(false);
            continue;
        }

        // 更新
        if (!emitter.pEmitterPtr->IsActive())
            emitter.pEmitterPtr->Update();
        else
            emitter.pEmitterPtr->SetActive(true);
    }
}

ParticleEmitter* Effect::AddEmitter(const std::string& _name,  float _delayTime, float _duration, bool _loop)
{
    EmitterData& emitter = emitters_.emplace_back();
    emitter.name = _name;
    emitter.pEmitterPtr = std::make_unique<ParticleEmitter>();
    emitter.delayTime = _delayTime;
    emitter.duration = _duration;
    emitter.loop = _loop;

    emitter.pEmitterPtr->Setting(_name);



    return emitter.pEmitterPtr.get();
}

std::list<ParticleEmitter*> Effect::GetEmitters() const
{
    std::list<ParticleEmitter*> list;

    for (const EmitterData& emitter : emitters_)
    {
        list.push_back(emitter.pEmitterPtr.get());
    }

    return list;
}

void Effect::Save()
{
}
