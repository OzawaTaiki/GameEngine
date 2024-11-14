#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <d3d12.h>
#include <wrl.h>
#include <initializer_list>

class WorldTransform
{
public:

    WorldTransform() = default;
    ~WorldTransform() = default;

    void Initialize();

    void UpdateData();
    void UpdateData(const std::initializer_list<Matrix4x4>& _mat);
    void TransferData();

    void QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const;

    ID3D12Resource* GetResource() const { return resource_.Get(); }

    Vector3 GetWorldPosition()const;


    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f ,0.0f };
    Vector3 transform_ = { 1.0f,1.0f ,1.0f };
    Matrix4x4 matWorld_ = {};

    const WorldTransform* parent_ = nullptr;

    // 自転
    // UpdateMatrix
    //
    // 公転 アンカー基準で回転させる
    // アンカー座標 pos 回転rotををせっと
    // transform(pos,world)*matRot  アンカーでの回転を適用
    // transform(-pos, world)       元の位置に戻す（たぶん
    //
    // 親の行列適用


private:


    struct DataForGPU
    {
        Matrix4x4 World;
        Matrix4x4 worldInverseTranspose;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    DataForGPU* constMap_;

};
