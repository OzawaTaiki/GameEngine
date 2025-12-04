#include "Resources/Shader/FullScreen.hlsli"

// 元画像
Texture2D<float4> gOriginalTexture : register(t0);
// ブラー済みテクスチャ
Texture2D<float4> gBlurTexture : register(t1);
SamplerState gSampler : register(s0);

cbuffer BloomConstants : register(b0)
{
    float threshold;  // 閾値（このシェーダーでは未使用）
    float intensity;  // ブルームの強度
    float softKnee;   // ソフトニー（このシェーダーでは未使用）
    float pad;
};

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexOutput input)
{
    PixelShaderOutput output;

    // 元画像をサンプリング
    float4 originalColor = gOriginalTexture.Sample(gSampler, input.uv);

    // ブラー済みテクスチャをサンプリング
    float4 blurColor = gBlurTexture.Sample(gSampler, input.uv);

    // 加算合成：元画像 + (ブラー × 強度)
    output.color.rgb = originalColor.rgb + blurColor.rgb * intensity;
    output.color.a = originalColor.a;

    return output;
}
