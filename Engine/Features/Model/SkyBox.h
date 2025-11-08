#pragma once

#include <Features/Model/ObjectModel.h>

class Camera;

class SkyBox
{
public:
    SkyBox() = default;
    ~SkyBox() = default;

    void Initialize(const Vector3& _scale, bool _useQuaternion = true);
    void Update();
    void Draw(const Camera* _camera);

    void SetColor(const Vector4& _color);

    void SetPosition(const Vector3& _position);
    void SetScale(const Vector3& _scale);
    void SetRotation(const Vector3& _rotation);
    void SetRotation(const Quaternion& _rotation);

    void SetTexture(const std::string& _name);
    void SetTexture(uint32_t _handle);

    void QueueCmdCubeTexture(uint32_t _index = 8) const;

private:
    void UpdateWorldTransform();

private:
    const uint32_t kIndexNum = 36;

    WorldTransform worldTransform_;
    ObjectColor objectColor_;

    ID3D12PipelineState* pipelineState_ = nullptr;
    ID3D12RootSignature* rootSignature_ = nullptr;

    Vector4 color_ = { 1,1,1,1 };

    uint32_t textureHandle_ = 0;

    bool isDirty_ = false;
    bool useQuaternion_ = false;

};