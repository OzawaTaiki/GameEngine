#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// scale = 16.0f
// power = 0.8f;

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexOutput _input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, _input.uv);

    float2 corrent = _input.uv * (1.0f - _input.uv.yx);

    float vignette = corrent.x * corrent.y * 16.0f;

    vignette = saturate(pow(vignette, 0.8f));

    output.color.rgb *= vignette;

    output.color.a = 1.0f;

    return output;
}