#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <d3d12.h>
#include <wrl.h>


class WorldTransform
{
public:

    WorldTransform() = default;
    ~WorldTransform() = default;

    void Initialize();

    void TransferData(const Matrix4x4& _viewProjectoin);

    ID3D12Resource* GetResource() const { return resource_.Get(); }

    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f ,0.0f };
    Vector3 transform_ = { 1.0f,1.0f ,1.0f };

private:
    Matrix4x4 matWorld_ = {};

    struct DataForGPU
    {
        Matrix4x4 World;
        Matrix4x4 WVP;
        Matrix4x4 worldInverseTranspose;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    DataForGPU* constMap_;

};