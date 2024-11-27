#include "ModelAnimation.h"
#include "MyLib.h"
#include "../Joint/joint.h"
#include <assimp/scene.h>
#include "ImGuiManager.h"
#include <cassert>

void ModelAnimation::Initialize()
{
    animetionTImer_ = 0.0f;
    localMatrix_ = MakeIdentity4x4();
}

void ModelAnimation::Update(std::vector<Joint>& _joints)
{
    animetionTImer_ += 1.0f / 60.0f;
    animetionTImer_ = std::fmod(animetionTImer_, animation_.duration);

    for (Joint& joint : _joints)
    {
        if (auto it = animation_.nodeAnimations.find(joint.name_); it != animation_.nodeAnimations.end())
        {
            const NodeAnimation& nodeAnimation = it->second;
            QuaternionTransform transform = {};
            transform.translate = CalculateValue(nodeAnimation.translate, animetionTImer_);
            transform.rotation = CalculateValue(nodeAnimation.rotation, animetionTImer_);
            transform.scale = CalculateValue(nodeAnimation.scale, animetionTImer_);
            joint.SetTransform(transform);
        }
    }

    /*ImGui::Text("%f", animetionTImer_);
    NodeAnimation& rootAnimation = animation_.nodeAnimations[_rootNodeName];
    Vector3 translate = CalculateValue(rootAnimation.translate, animetionTImer_);
    Quaternion rotation = CalculateValue(rootAnimation.rotation, animetionTImer_);
    rotation.ShowData("q", false);
    Vector3 scale = CalculateValue(rootAnimation.scale, animetionTImer_);
    localMatrix_ = MakeAffineMatrix(scale, rotation, translate);*/
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