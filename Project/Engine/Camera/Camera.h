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
    void Update(bool _showImGui = true);
    void Draw();

    ID3D12Resource* GetResource()const { return resource_.Get(); }
    Matrix4x4 GetViewProjection()const { return matViewProjection_; }

    
    /// <summary>
    /// セットした行列を転送
    /// </summary>
    void TransferData();

    /// <summary>
   /// メンバ変数から行列を計算，転送
   /// </summary>
    void UpdateMatrix();

    /// <summary>
    /// カメラを揺らす
    /// </summary>
    /// <param name="_time">揺れる時間</param>
    /// <param name="_RangeMin">揺れる最小値</param>
    /// <param name="_RangeMax">揺れる最大値</param>
    void Shake(float _time, const Vector2& _RangeMin, const Vector2& _RangeMax);

    void ShakeParametaerSettingFromImGui();

    Vector3 GetShakeOffset()const { return shakeOffset_; }

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

    // シェイク用変数たち
    bool shaking_ = false;
    float shakeTime_ = 0.0f;
    float shakeTimer_ = 0.0f;
    Vector2 shakeRangeMin_ = { 0.0f,0.0f };
    Vector2 shakeRangeMax_ = { 0.0f,0.0f };
    Vector3 shakeOffset_ = { 0.0f,0.0f,0.0f };

    void UpdateShake();

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