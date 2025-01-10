#pragma once

#include <Physics/Math/Vector3.h>
#include <Physics/Math/Quaternion.h>
#include <Physics/Math/Matrix4x4.h>
#include <Physics/Math/QuaternionTransform.h>

#include <vector>
#include <map>
#include <string>


struct aiAnimation;

class Joint;
class ModelAnimation
{
public:

    ModelAnimation() = default;
    ~ModelAnimation() = default;

    void Initialize();
    void Update(std::vector<Joint>& _joints,float _deltaTime);
    void Draw();

    void ReadAnimation(const aiAnimation* _animation);
    void ToIdle(float _timeToIdle);

    void SetLoop(bool _loop) { isLoop_ = _loop; }

    Matrix4x4 GetLocalMatrix() const { return localMatrix_; }
    bool IsPlaying() const { return isPlaying_; }

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
    };
    struct Animation
    {
        float duration; //全体の尺
        std::map<std::string, NodeAnimation> nodeAnimations;
    };

    Animation animation_;
    Matrix4x4 localMatrix_;
    float animetionTimer_ = 0.0f;

    bool isLoop_ = false;
    bool isPlaying_ = false;
    bool toIdle_ = false;
    float timeToIdle_ = 0.0f;
    // idle状態になる前のアニメーションの状態
    QuaternionTransform beforeIdleTransform_ = {};

    Vector3 CalculateValue(const AnimationCurve<Vector3>& _curve, float _time);
    Quaternion CalculateValue(const AnimationCurve<Quaternion>& _curve, float _time);

};
