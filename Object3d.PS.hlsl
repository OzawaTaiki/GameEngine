#include "Object3d.hlsli" 

cbuffer Material : register(b0)
{
    float4 color;
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
    //output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    
    float4 textureColor = gTexture.Sample(gSampler, _input.texcoord);
    
    output.color = color * textureColor;
    
    return output;
}