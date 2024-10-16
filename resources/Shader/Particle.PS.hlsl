#include "Particle.hlsli"

cbuffer gMaterial : register(b0)
{
    float4 materialColor;
    float4x4 uvTransform;
    int enableLighting;
    float isVisible;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    float4 textureColor;

    //画像の有無
    if (isVisible == 1.0f)
    {
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), uvTransform);
        textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    }
    else
        textureColor = materialColor;

    output.color = materialColor * textureColor * _input.color;
    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}