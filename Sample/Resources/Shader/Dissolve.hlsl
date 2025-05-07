#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

cbuffer ConstantBuffer : register(b0)
{
    float threshold;
    float3 maskColor;

    int enableEdgeColor;
    float3 edgeColor;

    float detectRange;
    float3 pad;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float> gMaskTexture : register(t1);


struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexOutput _input)
{
    PixelShaderOutput output;
    float maskTextureColor = gMaskTexture.Sample(gSampler, _input.uv);

    if (maskTextureColor < threshold)
    {
        output.color = float4(maskColor, 1.0f);
    }
    else
    {
        output.color = gTexture.Sample(gSampler, _input.uv);
        if (enableEdgeColor)
        {
            float edge = 1.0f - smoothstep(threshold, threshold + detectRange, maskTextureColor);
            output.color.rgb += edge * edgeColor;
        }
    }


    output.color.a = 1.0f;
    return output;
}