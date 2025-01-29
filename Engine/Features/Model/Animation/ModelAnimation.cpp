#include <Features/Model/Animation/ModelAnimation.h>
#include <Math/MyLib.h>
#include <Features/Model/Animation/Joint/Joint.h>
#include <Features/Json/Loader/JsonLoader.h>
#include <Debug/ImGuiManager.h>

#include <assimp/scene.h>

#include <fstream>
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
            //TODO : 補間処理
            //it->second.interpolation ごとに補間関数を作成
            const NodeAnimation& nodeAnimation = it->second;
            QuaternionTransform transform = {};

            if (nodeAnimation.interpolation == "LINEAR")
            {
                transform.translate = CalculateValue_Linear(nodeAnimation.translate, animetionTimer_);
                transform.rotation = CalculateValue_Linear(nodeAnimation.rotation, animetionTimer_);
                transform.scale = CalculateValue_Linear(nodeAnimation.scale, animetionTimer_);
            }
            else if (nodeAnimation.interpolation == "STEP")
            {
                transform.translate = CalculateValue_Step(nodeAnimation.translate, animetionTimer_);
                transform.rotation = CalculateValue_Step(nodeAnimation.rotation, animetionTimer_);
                transform.scale = CalculateValue_Step(nodeAnimation.scale, animetionTimer_);
            }
            else if (nodeAnimation.interpolation == "CUBICSPLINE")
            {
                transform.translate = CalculateValue_Linear(nodeAnimation.translate, animetionTimer_);
                transform.rotation = CalculateValue_Linear(nodeAnimation.rotation, animetionTimer_);
                transform.scale = CalculateValue_Linear(nodeAnimation.scale, animetionTimer_);
            }

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

        uint32_t samplerIndex = channelToSampler_[channelIndex];
        std::string interpolation = samplers_[samplerIndex].interpolation;

        nodeAnimation.interpolation = interpolation;

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

void ModelAnimation::ReadSampler(const std::string&  _filepath)
{
    std::ifstream ifs(_filepath);

    if (!ifs.is_open())
        return;

    json j;
    ifs >> j;

    if (j.contains("animations"))
    {
        for (auto& animation : j["animations"])
        {
            if (animation.contains("samplers"))
            {
                for (auto& sampler : animation["samplers"])
                {
                    Sampler s;

                    if (sampler.contains("input"))
                        s.input = sampler["input"];

                    if (sampler.contains("output"))
                        s.output = sampler["output"];

                    if (sampler.contains("interpolation"))
                        s.interpolation = sampler["interpolation"];
                    else
                        s.interpolation = "LINEAR";

                    samplers_.push_back(s);
                }
            }
            if (animation.contains("channels"))
            {
                uint32_t channelIndex = 0;
                for (auto& channel : animation["channels"])
                {
                    if (channel.contains("sampler"))
                    {
                        uint32_t samplerIndex = channel["sampler"];
                        if (samplers_.size() <= samplerIndex)
                            continue;

                        channelToSampler_[channelIndex++] = samplerIndex;
                    }

                }
            }
        }
    }

}

void ModelAnimation::ToIdle(float _timeToIdle)
{
    animetionTimer_ = 0.0f;
    toIdle_ = true;
    timeToIdle_ = _timeToIdle;
}

void ModelAnimation::Reset()
{
    animetionTimer_ = 0.0f;
    isPlaying_ = false;
    toIdle_ = false;
    timeToIdle_ = 0.0f;
}

Vector3 ModelAnimation::CalculateValue_Linear(const AnimationCurve<Vector3>& _curve, float _time)
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
Quaternion ModelAnimation::CalculateValue_Linear(const AnimationCurve<Quaternion>& _curve, float _time)
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

Vector3 ModelAnimation::CalculateValue_Step(const AnimationCurve<Vector3>& _curve, float _time)
{
    assert(!_curve.keyframes.empty());

    if (_curve.keyframes.size() == 1 || _time <= _curve.keyframes[0].time)
    {
        return _curve.keyframes[0].value;
    }

    int32_t index = static_cast<int32_t>(_curve.keyframes.size() - 1);
    for (; index >= 0; index--)
    {
        if (_time >= _curve.keyframes[index].time)
        {
            return _curve.keyframes[index].value;
        }
    }

    return (*_curve.keyframes.rbegin()).value;
}

Quaternion ModelAnimation::CalculateValue_Step(const AnimationCurve<Quaternion>& _curve, float _time)
{
    assert(!_curve.keyframes.empty());

    if (_curve.keyframes.size() == 1 || _time <= _curve.keyframes[0].time)
    {
        return _curve.keyframes[0].value;
    }

    int32_t index = static_cast<int32_t>(_curve.keyframes.size() - 1);
    for (; index >= 0; index--)
    {
        if (_time >= _curve.keyframes[index].time)
        {
            return _curve.keyframes[index].value;
        }
    }

    return (*_curve.keyframes.rbegin()).value;
}

