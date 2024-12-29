#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "ObjectColor.h"
#include <Framework/Camera/Camera.h>

class ObjectModel
{
public:

    ObjectModel() = default;
    ~ObjectModel() = default;

    void Initialize(const std::string& _filePath, const std::string& _name);
    void Update();
    void Draw(const Camera* _camera ,const Vector4& _color);


    void SetModel(const std::string& _filePath);
    void SetParent(const WorldTransform* _parent) { worldTransform_.parent_ = _parent; }

    const WorldTransform* GetWorldTransform() { return &worldTransform_; }

    void UpdateUVTransform();

    Vector3 GetMin()const { return model_->GetMin(); }
    Vector3 GetMax()const { return model_->GetMax(); } 

    Vector3 translate_ = { 0,0,0 };
    Vector3 scale_ = { 1,1,1 };
    Vector3 rotate_ = { 0,0,0 };

    Vector2 uvTransform_ = { 0,0 };
    Vector2 uvScale_ = { 1,1 };
    float uvRotation_ = 0;


private:

    WorldTransform worldTransform_;
    std::unique_ptr<ObjectColor> objectColor_ = nullptr;
    Model* model_ = nullptr;
    std::string name_ = "";

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG


};
