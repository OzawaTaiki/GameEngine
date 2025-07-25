#pragma once

#include <Math/Vector/Vector4.h>

#include <Features/PostEffects/PostEffectBase.h>

struct VignetteData
{
    Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f }; // ビネットの色

    float scale = 16.0f;
    float power = 0.8f;

    float pad[2];
};

class Vignette : public PostEffectBase
{
public:
    Vignette() = default;
    ~Vignette() = default;

    void Initialize() override;

    void Apply(const std::string& input, const std::string& output) override;

    void SetData(VignetteData* _data);

private:

    void CreatePipelineState();
    void CreateRootSignature();

    void DataUpdate();

private:

    VignetteData* data_ = nullptr; // ビネットデータへのポインタ
};
