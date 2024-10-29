#include "Particle.hlsli"

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
        textureColor = materialColor;

    output.color = materialColor * textureColor * _input.color;
    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}