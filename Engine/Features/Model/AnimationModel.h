#pragma once

#include <Features/Model/Model.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Model/Color/ObjectColor.h>
#include <Features/Camera/Camera/Camera.h>
#include <System/Time/GameTime.h>

#include <string>

class AnimationModel
{
public:

    AnimationModel(const std::string& _name);
    ~AnimationModel();


    void Initialize(const std::string& _filePath);
    void Update();
    void Draw(const Camera* _camera, const Vector4& _color);
    void Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color);

    void SetAnimation(const std::string& _name, bool _isLoop = false);
    void ChangeAnimation(const std::string& _name, float _blendTime, bool _isLoop = false);
    void StopAnimation() { model_->StopAnimation(); }
    void ToIdle(float _timeToIdle) { model_->ToIdle(_timeToIdle); }
    bool IsAnimationEnd() { return model_->IsAllAnimationEnd(); }
    bool IsIdle() { return model_->IsIdle(); }

    Vector3 GetMin() const { return model_->GetMin(); }
    Vector3 GetMax() const { return model_->GetMax(); }
    Matrix4x4 GetSkeletonSpaceMatrix(uint32_t _index = 0) const { return model_->GetSkeletonSpaceMatrix(_index); }

    const WorldTransform* GetWorldTransform() { return &worldTransform_; }

    void LoadAnimation(const std::string& _filePath) { model_->LoadAnimation(_filePath); }

    UVTransform& GetUVTransform(uint32_t _index = 0) { return model_->GetUVTransform(_index); }
    void SetParent(const WorldTransform* _parent) { worldTransform_.parent_ = _parent; }
    void SetModel(const std::string& _filePath) { model_ = Model::CreateFromObj(_filePath); }

    void SetTimeChannel(const std::string& _channel) { timeChannel = _channel; }

    Vector3 translate_ = { 0,0,0 };
    Vector3 scale_ = { 1,1,1 };
    // オイラー
    Vector3 euler_ = { 0,0,0 };
    // クォータニオン
    Quaternion quaternion_ = { 0,0,0,1 };
    bool useQuaternion_ = false;


    void ImGui();
private:

    WorldTransform worldTransform_;
    std::unique_ptr<ObjectColor> objectColor_ = nullptr;
    Model* model_ = nullptr;

    std::string timeChannel = "default";
    GameTime* gameTime_ = nullptr;

    std::string name_ = "";

};
