#include "Particle.hlsli"

struct ParticleForGPU
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput _input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(_input.position, gParticle[instanceID].WVP);
    output.texcoord = _input.texcoord;
    output.color = gParticle[instanceID].color;
    return output;
}



