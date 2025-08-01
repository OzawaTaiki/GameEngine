//#include "Resources/Shader/Object3d.hlsli"

cbuffer TransformationMatrix : register(b0)
{
    float4x4 World;
    float4x4 worldInverseTranspose;
};


struct DirectionalLight
{
    float4x4 lightVP;

    float4 color;

    float3 direction;
    float intensity;

    int isHalf;
    int castShadow;
    float shadowFactor;
    float pad;
};

struct PointLight
{
    float4 color;

    float3 position;
    float intensity;

    float radius;
    float decay;
    int isHalf;
    int castShadow;

    float shadowFactor;
    float3 pad;

    float4x4 lightVP[6];
};

struct SpotLight
{
    float4 color;

    float3 position;
    float intensity;

    float3 direction;
    float distance;

    float decay;
    float cosAngle;
    float cosFalloutStart;
    int isHalf;

    int castShadow;
    float shadowFactor;
    float2 pad;

    float4x4 lightVP;
};
static const int MAX_POINT_LIGHT = 32;
static const int MAX_SPOT_LIGHT = 32;
cbuffer gLightGroup : register(b1)
{
    DirectionalLight DL;
    PointLight PL[MAX_POINT_LIGHT];
    SpotLight SL[MAX_SPOT_LIGHT];
    int numPointLight;
    int numSpotLight;

}
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PixelShaderOutput
{
    float4 data : SV_TARGET0;
};

VertexShaderOutput ShadowMapVS(VertexShaderInput _input)
{

    VertexShaderOutput output;
    output.position = mul(_input.position, mul(World, DL.lightVP));

    return output;
}

PixelShaderOutput ShadowMapPS(VertexShaderOutput _input)
{
    PixelShaderOutput output;

    float z = _input.position.z / _input.position.w; // NDC座標のZ値を取得

    output.data = float4(z, z, z, 1.0f);
    return output;
}