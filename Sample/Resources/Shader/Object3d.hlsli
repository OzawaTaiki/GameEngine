struct VertexShaderOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : Position0;
};

cbuffer Camera : register(b0)
{
    float4x4 matView;
    float4x4 matProj;
    float3 worldPosition;
}
