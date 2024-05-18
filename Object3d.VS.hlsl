#include "Object3d.hlsli"

cbuffer TrancsFormationMatrix : register(b0)
{
    float4x4 WVP;
};

struct VertexShaderInput
{
    //float32_t4 position : POSITION0;
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput _input)
{
    VertexShaderOutput output;
    output.position = mul(_input.position, WVP);
    output.texcoord = _input.texcoord;
    return output;
}



