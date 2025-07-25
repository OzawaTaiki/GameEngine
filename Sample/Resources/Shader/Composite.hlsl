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

    if (output.color.a == 0)
        discard;

    output.color.a = 1.0f; // Set alpha to 1.0 for full opacity
    return output;
}