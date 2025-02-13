#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Math/Quaternion/QuaternionTransform.h>

#include <vector>
#include <map>
#include <string>


struct aiAnimation;

class Joint;
class ModelAnimation
{
private:
    template <typename T>
    struct Keyframe
    {
        float time;
        T value;
    };
    using KeyframeVector3 = Keyframe<Vector3>;
    using KeyframeQuaternion = Keyframe<Quaternion>;

    template <typename T>
    struct AnimationCurve
    {
        std::vector<Keyframe<T>> keyframes;
    };
    struct NodeAnimation
    {
        AnimationCurve<Vector3> translate;
        AnimationCurve<Quaternion> rotation;
        AnimationCurve<Vector3> scale;
        std::string interpolation;
    };
    struct Animation
    {
        float duration; //全体の尺
        std::map<std::string, NodeAnimation> nodeAnimations;
    };
public:

    ModelAnimation() = default;
    ~ModelAnimation() = default;

    void Initialize();
    void Update(std::vector<Joint>& _joints,float _deltaTime);
    void Draw();

    void ReadAnimation(const aiAnimation* _animation);
    void ReadSampler(const std::string& _filepath);

    void ToIdle(float _timeToIdle);
    void Reset();

    void SetLoop(bool _loop) { isLoop_ = _loop; }

    void ChangeAnimation(const Animation& _animation,float _blendTime);
    void SetAnimation(const Animation& _animation) { animation_ = _animation; }

    Animation GetAnimation() const { return animation_; }
    

    Matrix4x4 GetLocalMatrix() const { return localMatrix_; }
    bool IsPlaying() const { return isPlaying_; }
    bool IsIdle() const { return !toIdle_ && !isPlaying_; }

private:
    


    Animation animation_;
    Matrix4x4 localMatrix_;
    float animetionTimer_ = 0.0f;


    struct Sampler
    {
        uint32_t input;
        uint32_t output;
        std::string interpolation;
    };

    std::vector<Sampler> samplers_;
    // チャンネルとサンプラーの対応
    std::map<uint32_t, uint32_t> channelToSampler_;


    bool isLoop_ = false;
    bool isPlaying_ = false;
    bool toIdle_ = false;
    float timeToIdle_ = 0.0f;

    struct AnimationState
    {
        std::map<std::string, QuaternionTransform> lastPose;
        float blendTime = 0.0f;                        // ブレンド経過時間
        float totalBlendTime = 0.0f;                   // ブレンド完了までの時間
        bool isBlending = false;

    };

    AnimationState state_;


    Vector3 CalculateValue_Linear(const AnimationCurve<Vector3>& _curve, float _time);
    Quaternion CalculateValue_Linear(const AnimationCurve<Quaternion>& _curve, float _time);

    Vector3 CalculateValue_Step(const AnimationCurve<Vector3>& _curve, float _time);
    Quaternion CalculateValue_Step(const AnimationCurve<Quaternion>& _curve, float _time);

};
