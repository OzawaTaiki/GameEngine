#include "Resources/Shader/Object3dInstanced.hlsli"
//#include "Object3dInstanced.hlsli"

struct InstanceData
{
    float4x4 World;
    float4x4 worldInverseTranspose;
    float4 color;
};

StructuredBuffer<InstanceData> models : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput _input, uint instanceID : SV_InstanceID)
{
    InstanceData data = models[instanceID];

    VertexShaderOutput output;

    output.position = mul(_input.position, mul(data.World, mul(matView, matProj)));
    output.texcoord = _input.texcoord;
    output.normal = normalize(mul(_input.normal, (float3x3) data.worldInverseTranspose));
    output.worldPosition = mul(_input.position, data.World).xyz;
    output.shadowPos = mul(mul(_input.position, data.World), DL.lightVP);
    output.color = data.color;
    return output;
}
