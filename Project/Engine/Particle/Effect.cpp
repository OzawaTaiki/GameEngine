#include "Effect.h"
#include "Utility/Time.h"
#include "Utility/ConfigManager.h"


void Effect::Update()
{
    if (!isActive_)
        return;

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

void Effect::AddEmitter(const std::string& _name, ParticleEmitter* _pEmitter, float _delayTime, float _duration, bool _loop)
{
    EmitterData data;
    data.pEmitterPtr = _pEmitter;
    data.delayTime = _delayTime;
    data.duration = _duration;
    data.loop = _loop;
    emitters_.push_back(data);
}

void Effect::Save()
{
}
