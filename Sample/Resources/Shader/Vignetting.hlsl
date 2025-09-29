#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer KernelSizeBuffer : register(b0)
{
    float4 vignetteColor; // ビネットカラー

    float scale; // スケール（強度）
    float power; // パワー（減衰カーブ）
    float2 pad; // パディング
}
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexOutput _input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, _input.uv);

    float2 corrent = _input.uv * (1.0f - _input.uv.yx);

    float vignette = corrent.x * corrent.y * scale;

    vignette = saturate(pow(vignette, power));

    output.color.rgb = lerp(vignetteColor.rgb, output.color.rgb, vignette);

    return output;
}