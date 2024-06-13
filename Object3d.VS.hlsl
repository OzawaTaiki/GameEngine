#include "Object3d.hlsli"

cbuffer TrancsFormationMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput _input)
{
    VertexShaderOutput output;
    output.position = mul(_input.position, WVP);
    output.texcoord = _input.texcoord;
    output.normal = normalize(mul(_input.normal, (float3x3) World));
    return output;
}



