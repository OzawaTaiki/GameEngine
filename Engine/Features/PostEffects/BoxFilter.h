#pragma once

#include <Features/PostEffects/PostEffectBase.h>

struct BoxFilterData
{
    int kernelSize = 5; // カーネルサイズ

    float padding[3] = { 0.0f, 0.0f, 0.0f }; // パディング用
};

class BoxFilter : public PostEffectBase
{
public:
    BoxFilter() = default;
    ~BoxFilter() override = default;

    void Initialize() override;

    void Apply(const std::string& input, const std::string& output) override;

    void SetData(const BoxFilterData* data);

private:

    void CreatePipelineState();

    void CreateRootSignature();

private:


};