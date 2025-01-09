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
static const int MAX_POINT_LIGHT = 32;
static const int MAX_SPOT_LIGHT = 16;
cbuffer gLightGroup : register(b3)
{
    DirectionalLight DL;
    PointLight PL[MAX_POINT_LIGHT];
    SpotLight SL[MAX_SPOT_LIGHT];
    int numPointLight;
    int numSpotLight;
}

////平行光源
//cbuffer gDirectionalLight : register(b3)
//{
//    DirectionalLight DL;
//}

////点光源
//cbuffer gPointLight : register(b4)
//{
//    PointLight PL;
//}

////スポットライト
//cbuffer gSpotLight : register(b5)
//{
//    SpotLight SL;
//}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, float3 _toEye, float4 _textureColor);
float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _textureColor);

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 textureColor;

    float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
    textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);

    float3 toEye = normalize(worldPosition - _input.worldPosition);

    float3 directionalLight = CalculateDirectionalLighting(_input, toEye, textureColor);
    float3 pointLight = CalculateLightingWithMultiplePointLights(_input, toEye, textureColor);
    float3 spotLightcColor = CalculateLightingWithMultipleSpotLights(_input, toEye, textureColor);

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

float3 CalculatePointLighting(VertexShaderOutput _input,PointLight _PL, float3 _toEye, float4 _textureColor)
{
    if (_PL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    float3 direction = normalize(_input.worldPosition - _PL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);
    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (_PL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float distance = length(_PL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _PL.radius + 1.0f), _PL.decay);
    float3 diffuse = materialColor.rgb * _textureColor.rgb * _PL.color.rgb * cos * _PL.intensity * factor;
    float3 specular = _PL.color.rgb * _PL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor;

    return diffuse + specular;
}

float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _textureColor)
{
    if (_SL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);


    float3 direction = normalize(_input.worldPosition - _SL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);

    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (_SL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }

    float distance = length(_SL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _SL.distance + 1.0f), _SL.decay);

    float cosAngle = dot(direction, normalize(_SL.direction));
    float falloffFactor = 1.0f;
    if (cosAngle < _SL.cosFalloutStart)
    {
        falloffFactor = saturate((cosAngle - _SL.cosAngle) / (_SL.cosFalloutStart - _SL.cosAngle));
    }


    float3 diffuse = materialColor.rgb * _textureColor.rgb * _SL.color.rgb * cos * _SL.intensity * factor * falloffFactor;
    float3 specular = _SL.color.rgb * _SL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular;

}

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numPointLight; i++)
    {
        lighting += CalculatePointLighting(_input, PL[i], _toEye, _textureColor);;
    }
    return lighting;
}

float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numSpotLight; i++)
    {
        lighting += CalculateSpotLighting(_input, SL[i], _toEye, _textureColor);
    }
    return lighting;
}
