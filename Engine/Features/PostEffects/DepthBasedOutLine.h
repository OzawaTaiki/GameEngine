#pragma once

#include <Features/PostEffects/PostEffectBase.h>

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>

struct DepthBasedOutLineData
{
    Matrix4x4 inverseViewProjectionMatrix; // カメラの逆射影行列

    float edgeThreshold = 0.5f; // エッジの閾値 (0.0~10.0くらい)
    float edgeIntensity = 0.5f; // エッジの強度 (0.0~1.0)
    float edgeWidth = 1.0f; // エッジの幅 (1.0~3.0くらい)
    int enableColorBlending = 1; // カラーブレンディング有効（0:無効, 1:有効）

    Vector3 edgeColor = { 1.0f, 1.0f, 1.0f }; // エッジの色（RGB）
    float edgeColorIntensity = 1.0f; // エッジ色の強度（0.0~1.0）

    void ImGui();
};

class Camera;
class DepthBasedOutLine : public PostEffectBase
{
public:
    void Initialize() override;

    void Apply(const std::string& _input, const std::string& _output) override;

    void SetData(DepthBasedOutLineData* _data);

    void SetCamera(Camera* camera);

private:
    void CreatePipelineState();

    void CreateRootSignature();

    void UpdateData();
private:

    Camera* camera_ = nullptr; // カメラへのポインタ

    DepthBasedOutLineData* data_ = nullptr; // データへのポインタ
};