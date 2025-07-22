#pragma once

#include <Features/PostEffects/PostEffectBase.h>

struct BoxFilterData : public PostEffectBaseData
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

    void SetData(const PostEffectBaseData* data) override;

private:

    void CreatePipelineState();

    void CreateRootSignature();

private:


};