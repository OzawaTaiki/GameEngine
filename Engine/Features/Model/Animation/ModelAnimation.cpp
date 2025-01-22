#include <Features/Model/Animation/ModelAnimation.h>
#include <Math/MyLib.h>
#include <Features/Model/Animation/Joint/Joint.h>
#include <Debug/ImGuiManager.h>

#include <assimp/scene.h>
#include <cassert>

void ModelAnimation::Initialize()
{
    animetionTimer_ = 0.0f;
    localMatrix_ = MakeIdentity4x4();
}

void ModelAnimation::Update(std::vector<Joint>& _joints, float _deltaTime)
{
    isPlaying_ = true;
    animetionTimer_ += _deltaTime;

    if (toIdle_)
    {
        if (animetionTimer_ >= timeToIdle_)
        {
            animetionTimer_ = 0.0f;
            toIdle_ = false;
            isPlaying_ = false;
            for (Joint& joint : _joints)
            {
                joint.SetTransform(joint.GetIdleTransform());
            }
            return;
        }

        for (Joint& joint : _joints)
        {
            float t = animetionTimer_ / timeToIdle_;
            QuaternionTransform transform = {};
            QuaternionTransform idleTransform = joint.GetIdleTransform();
            transform.translate = Lerp(joint.GetTransform().translate, idleTransform.translate, t);
            transform.rotation = Slerp(joint.GetTransform().rotation, idleTransform.rotation, t);
            transform.scale = Lerp(joint.GetTransform().scale, idleTransform.scale, t);

            joint.SetTransform(transform);
        }
        return;
    }

    // ループしない場合
    if (!isLoop_)
    {
        // 再生時間がアニメーションの尺を超えたら再生を止める
        if (animetionTimer_ >= animation_.duration)
        {
            animetionTimer_ = animation_.duration;
            isPlaying_ = false;
        }
    }
    else
        animetionTimer_ = std::fmod(animetionTimer_, animation_.duration);

    for (Joint& joint : _joints)
    {
        if (auto it = animation_.nodeAnimations.find(joint.name_); it != animation_.nodeAnimations.end())
        {
            const NodeAnimation& nodeAnimation = it->second;
            QuaternionTransform transform = {};
            transform.translate = CalculateValue(nodeAnimation.translate, animetionTimer_);
            transform.rotation = CalculateValue(nodeAnimation.rotation, animetionTimer_);
            transform.scale = CalculateValue(nodeAnimation.scale, animetionTimer_);
            joint.SetTransform(transform);
        }
    }

    if (!isPlaying_)
        Initialize();

}

void ModelAnimation::Draw()
{
}

void ModelAnimation::ReadAnimation(const aiAnimation* _animation)
{
    animation_.duration = static_cast<float> (_animation->mDuration / _animation->mTicksPerSecond);

    for (uint32_t channelIndex = 0; channelIndex < _animation->mNumChannels; ++channelIndex)
    {
        aiNodeAnim* aiNodeAnimation = _animation->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = animation_.nodeAnimations[aiNodeAnimation->mNodeName.C_Str()];

        for (uint32_t keyframeIndex = 0; keyframeIndex < aiNodeAnimation->mNumPositionKeys; ++keyframeIndex)
        {
            aiVectorKey& aiKeyframe = aiNodeAnimation->mPositionKeys[keyframeIndex];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float> (aiKeyframe.mTime / _animation->mTicksPerSecond);
            keyframe.value = Vector3(-aiKeyframe.mValue.x, aiKeyframe.mValue.y, aiKeyframe.mValue.z);
            nodeAnimation.translate.keyframes.push_back(keyframe);
        }
        for (uint32_t keyframeIndex = 0; keyframeIndex < aiNodeAnimation->mNumRotationKeys; ++keyframeIndex)
        {
            aiQuatKey& aiKeyframe = aiNodeAnimation->mRotationKeys[keyframeIndex];
            KeyframeQuaternion keyframe;
            keyframe.time = static_cast<float> (aiKeyframe.mTime / _animation->mTicksPerSecond);
            keyframe.value = Quaternion(aiKeyframe.mValue.x, -aiKeyframe.mValue.y, -aiKeyframe.mValue.z, aiKeyframe.mValue.w).Normalize();
            nodeAnimation.rotation.keyframes.push_back(keyframe);
        }
        for (uint32_t keyframeIndex = 0; keyframeIndex < aiNodeAnimation->mNumScalingKeys; ++keyframeIndex)
        {
            aiVectorKey& aiKeyframe = aiNodeAnimation->mScalingKeys[keyframeIndex];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float> (aiKeyframe.mTime / _animation->mTicksPerSecond);
            keyframe.value = Vector3(aiKeyframe.mValue.x, aiKeyframe.mValue.y, aiKeyframe.mValue.z);
            nodeAnimation.scale.keyframes.push_back(keyframe);
        }
    }
    Initialize();
}

void ModelAnimation::ToIdle(float _timeToIdle)
{
    animetionTimer_ = 0.0f;
    toIdle_ = true;
    timeToIdle_ = _timeToIdle;
}


Vector3 ModelAnimation::CalculateValue(const AnimationCurve<Vector3>& _curve, float _time)
{
    assert(!_curve.keyframes.empty());

    // キーが一つか最初のキーフレームより前
    if (_curve.keyframes.size() == 1 || _time <= _curve.keyframes[0].time)
    {
        return _curve.keyframes[0].value;
    }

    for (size_t index = 0; index < _curve.keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        if (_curve.keyframes[index].time <= _time && _time <= _curve.keyframes[nextIndex].time)
        {
            float t = (_time - _curve.keyframes[index].time) / (_curve.keyframes[nextIndex].time - _curve.keyframes[index].time);
            return Lerp(_curve.keyframes[index].value, _curve.keyframes[nextIndex].value, t);
        }
    }

    return (*_curve.keyframes.rbegin()).value;
}
Quaternion ModelAnimation::CalculateValue(const AnimationCurve<Quaternion>& _curve, float _time)
{
    assert(!_curve.keyframes.empty());

    // キーが一つか最初のキーフレームより前
    if (_curve.keyframes.size() == 1 || _time <= _curve.keyframes[0].time)
    {
        return _curve.keyframes[0].value;
    }

    for (size_t index = 0; index < _curve.keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        if (_curve.keyframes[index].time <= _time && _time <= _curve.keyframes[nextIndex].time)
        {
            float t = (_time - _curve.keyframes[index].time) / (_curve.keyframes[nextIndex].time - _curve.keyframes[index].time);
            return Slerp(_curve.keyframes[index].value, _curve.keyframes[nextIndex].value, t);
        }
    }

    return (*_curve.keyframes.rbegin()).value;
}
