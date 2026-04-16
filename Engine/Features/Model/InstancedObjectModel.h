#pragma once
#include <Features/Model/Model.h>
#include <Features/Camera/Camera/Camera.h>

#include <cstdint>
#include <string>

#include <wrl.h>
#include <d3d12.h>

namespace Engine
{

/// <summary>
/// インスタンシング描画用のモデルクラス
/// アニメーションは非対応で、単純に複数のインスタンスを描画するためのクラス
/// </summary>
class InstancedObjectModel
{
public:

    InstancedObjectModel() = default;
    ~InstancedObjectModel();

    static constexpr uint32_t kDefaultMaxInstances = 1024;

    void Initialize(const std::string& modelPath, uint32_t maxInstances = kDefaultMaxInstances);

    void AddInstance(const Matrix4x4& worldMatrix, const Vector4& color = {1.0f,1.0f ,1.0f ,1.0f });

    void Draw(const Camera* camera);

    void Clear();

    uint32_t GetInstanceCount() const { return instanceCount_; }
private:

    struct InstanceData
    {
        Matrix4x4 world;
        Matrix4x4 worldInverseTranspose;
        Vector4   color;
    };

private:

    Model* model_ = nullptr;
    uint32_t maxInstances_ = 0;
    uint32_t instanceCount_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
    InstanceData* instanceMap_ = nullptr;

    static constexpr uint32_t kInvalidIndex = 0xFFFFFFFF;
    uint32_t srvIndex_ = kInvalidIndex;
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_ = {};

};


}// namespace Engine