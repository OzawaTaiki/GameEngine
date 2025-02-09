#pragma once

#include <Features/Animation/Sequence/SequenceEvent.h>

#include <list>
#include<memory>

class WorldTransform;
class AnimationSequence
{
public:
    AnimationSequence();
    ~AnimationSequence();
    void Update();


    void AddTargetWorldTransform(WorldTransform* _worldTransform);

private:

    float currentTime_;

    // 値を適用するワールドトランスフォーム
    std::list<WorldTransform*> targetWorldTransforms_;
    std::list<std::unique_ptr<SequenceEvent>>sequenceEvents_;

};
