#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float2 gCenter = float2(0.5f, 0.5f); // 中心座標
static const float kBulrWidth = 0.01f; // 大きいほどぼかしが強くなる

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};


PixelShaderOutput main(VertexOutput _input)
{
    static const int kSampleCount = 10; // サンプル数

    float2 direction = _input.uv - gCenter; // 中心からの方向ベクトル
    float3 sampleColor = float3(0.0f, 0.0f, 0.0f); // サンプルカラー

    for (int i = 0; i < kSampleCount; i++)
    {
        float2 texcoord = _input.uv + direction * kBulrWidth * float(i);
        sampleColor += gTexture.Sample(gSampler, texcoord).rgb; // サンプルカラーを加算
    }

    sampleColor *= rcp(float(kSampleCount));


    PixelShaderOutput output;
    output.color.rgb = sampleColor;
    output.color.a = 1.0f;
    return output;
}