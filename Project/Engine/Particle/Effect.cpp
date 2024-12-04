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
        for (auto& emitter : emitters_)
        {
            emitter.Reset();
        }
        return;
    }

    elapsedTime_ += 1.0f / 60.0f;
    //elapsedTime_ += Time::GetDeltaTime<float>();

    for (auto& emitter : emitters_)
    {
        // 遅延時間が経過していなかったらスキップ
        if (elapsedTime_ <= emitter.GetDelayTime())
        {
            emitter.SetActive(false);
            continue;
        }
        if (elapsedTime_ >= emitter.GetDuration() + emitter.GetDelayTime())
        {
            emitter.SetAlive(false);
        }


        if (!emitter.IsActive() && emitter.IsAlive())
            emitter.SetActive(true);

        // 更新
        emitter.Update();
    }
}

ParticleEmitter* Effect::AddEmitter(const std::string& _name)
{
    ParticleEmitter& emitter = emitters_.emplace_back();

    emitter.Setting(_name);


    return &emitter;
}

std::list<ParticleEmitter*> Effect::GetEmitters() const
{
    std::list<ParticleEmitter*> list;

    for (const ParticleEmitter& emitter : emitters_)
    {
        list.push_back(const_cast<ParticleEmitter*>(&emitter));
    }

    return list;
}

void Effect::Save()
{
}
