#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexOutput _input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, _input.uv);
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    output.color.rgb = float3(value, value, value);
    output.color.a = 1.0f;
    return output;
}

