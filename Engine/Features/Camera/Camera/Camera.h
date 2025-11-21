#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>
#include <System/Time/GameTime.h>
#include <Core/WinApp/WinApp.h>

#include <wrl.h>
#include <d3d12.h>

enum class CameraType
{
    Perspective,
    Orthographic
};

class Camera
{
public:

    Camera() = default;
    ~Camera() = default;

    void Initialize(CameraType _cameraType = CameraType::Perspective, const Vector2& _winSize = WinApp::kWindowSize_);
    void Update();
    void Draw();

    ID3D12Resource* GetResource()const { return resource_.Get(); }
    Matrix4x4 GetViewProjection()const { return matViewProjection_; }

    Vector2 WotldToScreen(const Vector3& _worldPos) const;


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

    void SetTimeChannel(const std::string& _name) { timeChannel_ = _name; }


    Vector3 scale_ = { 1.0f,1.0f ,1.0f };
    Vector3 rotate_ = { 0.0f,0.0f ,0.0f };
    Vector3 translate_ = { 0.0f,0.0f ,-20.0f };

    float fovY_ = 0.45f;
    float aspectRatio_ = WinApp::kWindowSize_.x / WinApp::kWindowSize_.y;
    float nearClip_ = 0.1f;
    float farClip_ = 1000.0f;


    Matrix4x4 matView_ = {};
    Matrix4x4 matProjection_ = {};

    // 透視投影か正射影投影か
    CameraType cameraType_ = CameraType::Perspective;

    void ImGui();
private:


private:
    // ortho用
    Vector2 LeftTop_ = { 0.0f, 0.0f }; // 左上座標
    Vector2 RightBottom_ = WinApp::kWindowSize_; // 右下座標
    Vector2 winSize_ = WinApp::kWindowSize_; // 正射影投影のサイズ

    // シェイク用変数たち
    bool shaking_ = false;
    float shakeTime_ = 0.0f;
    float shakeTimer_ = 0.0f;
    Vector2 shakeRangeMin_ = { 0.0f,0.0f };
    Vector2 shakeRangeMax_ = { 0.0f,0.0f };
    Vector3 shakeOffset_ = { 0.0f,0.0f,0.0f };

    std::string timeChannel_ = "default";
    GameTime* gameTime_ = nullptr;

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

    Matrix4x4 LoolAt(const Vector3& _eye, const Vector3& _at, const Vector3& _up);
};


