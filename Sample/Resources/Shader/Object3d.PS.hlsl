#include "Resources/Shader/Object3d.hlsli"
//#include "Object3d.hlsli"


cbuffer gMaterial : register(b1)
{
    float4x4 unTransform;
    float shininess;
    int enableLighting;
};

//cbuffer gTexVisibility : register(b1)
//{
//    float isVisible;
//};

cbuffer gColor : register(b2)
{
    float4 materialColor;
}

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    int isHalf;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    int isHalf;
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
};

//平行光源
cbuffer gDirectionalLight : register(b3)
{
    DirectionalLight DL;
}

//点光源
cbuffer gPointLight : register(b4)
{
    PointLight PL;
}

//スポットライト
cbuffer gSpotLight : register(b5)
{
    SpotLight SL;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculatePointLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculateSpotLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 textureColor;

    float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
    textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);

    float3 toEye = normalize(worldPosition - _input.worldPosition);

    float3 directionalLight = CalculateDirectionalLighting(_input, toEye, textureColor);
    float3 pointLight = CalculatePointLighting(_input, toEye, textureColor);
    float3 spotLightcColor = CalculateSpotLighting(_input, toEye, textureColor);

    if (enableLighting != 0)
    {
        output.color.rgb = directionalLight + pointLight + spotLightcColor;
        output.color.a = materialColor.a * textureColor.a;
    }
    else
        output.color = materialColor * textureColor;

    if (textureColor.a == 0.0 ||
        output.color.a == 0.0)
    {
        discard;
    }

    return output;
}

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    if (DL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);
    float3 HalfVector = normalize(-DL.direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);
    float NdotL = dot(normalize(_input.normal), -DL.direction);
    float cos = saturate(NdotL);
    if (DL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float3 diffuse = materialColor.rgb * _textureColor.rgb * DL.color.rgb * cos * DL.intensity;
    float3 specular = DL.color.rgb * DL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
    return diffuse + specular;
}

float3 CalculatePointLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    if (PL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);
    float3 direction = normalize(_input.worldPosition - PL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);
    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (PL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float distance = length(PL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / PL.radius + 1.0f), PL.decay);
    float3 diffuse = materialColor.rgb * _textureColor.rgb * PL.color.rgb * cos * PL.intensity * factor;
    float3 specular = PL.color.rgb * PL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor;
    return diffuse + specular;
}

float3 CalculateSpotLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    if (SL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);


    float3 direction = normalize(_input.worldPosition - SL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);

    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (SL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }

    float distance = length(SL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / SL.distance + 1.0f), SL.decay);

    float cosAngle = dot(direction, normalize(SL.direction));
    float falloffFactor = 1.0f;
    if (cosAngle < SL.cosFalloutStart)
    {
        falloffFactor = saturate((cosAngle - SL.cosAngle) / (SL.cosFalloutStart - SL.cosAngle));
    }


    float3 diffuse = materialColor.rgb * _textureColor.rgb * SL.color.rgb * cos * SL.intensity * factor * falloffFactor;
    float3 specular = SL.color.rgb * SL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular;

}