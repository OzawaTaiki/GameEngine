#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Math/Quaternion/Quaternion.h>

#include <d3d12.h>
#include <wrl.h>
#include <initializer_list>

class WorldTransform
{
public:

    WorldTransform() = default;
    ~WorldTransform() = default;

    void Initialize();

    void UpdateData(bool _useQuaternion = false);
    void UpdateData(const std::initializer_list<Matrix4x4>& _mat);
    void TransferData();

    void QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const;

    ID3D12Resource* GetResource() const { return resource_.Get(); }

    Vector3 GetWorldPosition()const;


    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f ,0.0f };
    Vector3 transform_ = { 1.0f,1.0f ,1.0f };
    Quaternion quaternion_ = { 0,0,0,1 };
    Matrix4x4 matWorld_ = {};

    const WorldTransform* parent_ = nullptr;

private:


    struct DataForGPU
    {
        Matrix4x4 World;
        Matrix4x4 worldInverseTranspose;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    DataForGPU* constMap_;

};
