#pragma once
#include <Features/Model/Model.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Model/Color/ObjectColor.h>
#include <Math/Quaternion/Quaternion.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Model/Animation/Controller/AnimationController.h>

class ObjectModel
{
public:

    ObjectModel(const std::string& _name);
    ~ObjectModel();

    void Initialize(const std::string& _filePath);
    void Initialize(std::unique_ptr<Mesh> _mesh);
    void Initialize(Model* _model);

    void Update();

    void Draw(const Camera* _camera);
    void Draw(const Camera* _camera ,const Vector4& _color);
    void Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color);
    void DrawShadow(const Camera* _camera);


    void UseQuaternion(bool _use) { useQuaternion_ = _use; }

    void LoadAnimation(const std::string& _filePath, const std::string& _name);
    void SetAnimation(const std::string& _name, bool _isLoop = false);
    void ChangeAnimation(const std::string& _name, float _blendTime, bool _isLoop = false);

    bool IsEndAnimation() const { return !uniqueAnimationController_ || !uniqueAnimationController_->IsAnimationPlaying(); }

    void SetModel(const std::string& _filePath);
    void SetSharedAnimationController(AnimationController* _controller);
    void SetParent(const WorldTransform* _parent) { worldTransform_.SetParent(_parent); }
    void SetParent(const Matrix4x4* _parentMatrix) { worldTransform_.SetParent(_parentMatrix); }

    WorldTransform* GetWorldTransform() { return &worldTransform_; }

    UVTransform& GetUVTransform(uint32_t _index = 0) { return model_->GetUVTransform(_index); }
    Vector3 GetMin()const { return model_->GetMin(); }
    Vector3 GetMax()const { return model_->GetMax(); }
    Material* GetMaterial() { return model_->GetMaterialPtr(); }

    void SetTimeChannel(const std::string& _channelName) { timeChannel = _channelName; }

    std::string GetName() const{ return name_; }

    std::unique_ptr<AnimationController> GetAnimationController();


    const Matrix4x4* GetSkeletonSpaceMatrix(const std::string& _jointName)  const;

    Vector3 translate_ = { 0,0,0 };
    Vector3 scale_ = { 1,1,1 };
    Vector3 euler_ = { 0,0,0 };

    Quaternion quaternion_ = { 0,0,0,1 };
    bool useQuaternion_ = false;

    bool drawSkeleton_ = false; // スケルトンを描画するかどうか


    void ImGui();

private:
    void InitializeCommon(); // 共通初期化

    WorldTransform worldTransform_;

    std::unique_ptr<ObjectColor> objectColor_ = nullptr;
    std::unique_ptr<AnimationController> uniqueAnimationController_ = nullptr;
    AnimationController* sharedAnimationController_ = nullptr;

    Model* model_ = nullptr;
    std::string name_ = "";


    std::string timeChannel = "default";
    GameTime* gameTime_ = nullptr;

    char filePathBuffer_[128];

};