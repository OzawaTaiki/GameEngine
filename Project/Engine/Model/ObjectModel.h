#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "ObjectColor.h"
#include "Camera.h"
#include "../Collider/Collider.h"

class ObjectModel
{
public:

    ObjectModel() = default;
    ~ObjectModel() = default;

    void Initialize(const std::string& _filePath);
    void Update();
    void Draw(const Camera* _camera ,const Vector4& _color);

    void SetModel(const std::string& _filePath);

    Vector3 translate_ = { 0,0,0 };
    Vector3 scale_ = { 1,1,1 };
    Vector3 rotate_ = { 0,0,0 };


private:

    WorldTransform worldTransform_;
    std::unique_ptr<ObjectColor> objectColor_ = nullptr;
    Model* model_ = nullptr;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG


};