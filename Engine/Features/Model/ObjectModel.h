#pragma once
#include <Features/Model/Model.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Model/Color/ObjectColor.h>
#include <Math/Quaternion/Quaternion.h>
#include <Features/Camera/Camera/Camera.h>

class ObjectModel
{
public:

    ObjectModel() = default;
    ~ObjectModel() = default;

    void Initialize(const std::string& _filePath, const std::string& _name="");
    void Update();
    void Draw(const Camera* _camera ,const Vector4& _color);
    void Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color);
    void UseQuaternion(bool _use) { useQuaternion_ = _use; }


    void SetModel(const std::string& _filePath);
    void SetParent(const WorldTransform* _parent) { worldTransform_.parent_ = _parent; }

    const WorldTransform* GetWorldTransform() { return &worldTransform_; }

    void UpdateUVTransform();

    UVTransform& GetUVTransform(uint32_t _index = 0) { return model_->GetUVTransform(_index); }
    Vector3 GetMin()const { return model_->GetMin(); }
    Vector3 GetMax()const { return model_->GetMax(); }

    Vector3 translate_ = { 0,0,0 };
    Vector3 scale_ = { 1,1,1 };
    Vector3 euler_ = { 0,0,0 };

    Quaternion quaternion_ = { 0,0,0,1 };
    bool useQuaternion_ = false;


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG


private:

    WorldTransform worldTransform_;
    std::unique_ptr<ObjectColor> objectColor_ = nullptr;
    Model* model_ = nullptr;
    std::string name_ = "";



};
