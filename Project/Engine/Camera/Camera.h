#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <wrl.h>
#include <d3d12.h>

class Camera
{
public:

    Camera() = default;
    ~Camera() = default;

    void Initialize();
    void Update();
    void Draw();

    ID3D12Resource* GetResource()const { return resource_.Get(); }
    Matrix4x4 GetViewProjection()const { return matViewProjection_; }

    
    /// <summary>
    /// セットした行列を転送
    /// </summary>
    void TransferData();

    void QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index)const;


    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f ,0.0f };
    Vector3 translate_ = { 0.0f,0.0f ,-20.0f };

    float fovY_ = 0.45f;
    float aspectRatio_ = 16.0f / 9.0f;
    float nearClip_ = 0.1f;
    float farClip_ = 1000.0f;


    Matrix4x4 matView_ = {};
    Matrix4x4 matProjection_ = {};
private:
    struct ConstantBufferDate
    {
        Matrix4x4 view;
        Matrix4x4 proj;
        Vector3 pos;
    };

    Matrix4x4 matWorld_ = {};
    Matrix4x4 matViewProjection_ = {};

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    ConstantBufferDate* constMap_ = nullptr;

    void Map();
};