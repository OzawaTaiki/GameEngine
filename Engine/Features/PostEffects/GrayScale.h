#pragma once

#include <Features/PostEffects/PostEffectBase.h>

struct GrayScaleData
{
    float intensity = 1.0f; // グレースケールの強度

    float padding[3] = { 0.0f, 0.0f, 0.0f }; // パディング用
};

class GrayScale : public PostEffectBase
{
public:
    GrayScale() = default;
    ~GrayScale() override = default;

    void Initialize() override;

    void Apply(const std::string& _input, const std::string& _output) override;

    void SetData(GrayScaleData* _data);
private:

    void CreatePipelineState();
    void CreateRootSignature();

    void UpdateData();
private:

    GrayScaleData* data_; // グレースケールのデータ


};
