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

//平行光源
cbuffer gDirectionalLight : register(b3)
{
    float4 DL_color; //ライトの色
    float3 DL_direction; //ライトの向き
    float DL_intensity; //輝度
    int DL_isHalf;
}

//点光源
cbuffer gPointLight : register(b4)
{
    float4 PL_color;
    float3 PL_position;
    float PL_intensity;
    float PL_radius;
    float PL_decay;
    int PL_isHalf;
}

//スポットライト
cbuffer gSpotLight : register(b5)
{
    float4 SL_color;
    float3 SL_position;
    float SL_intensity;
    float3 SL_direction;
    float SL_ditance;
    float SL_decay;
    float SL_cosAngle;
    float SL_cosFalloutStart;
    int SL_isHalf;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float3 CalculateSpotLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 textureColor;

    //画像の有無
    //if (isVisible == 1.0f)
    //{
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
        textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);
    //}
    //else
    //    textureColor = materialColor;

    /*lmbart*/
    float3 toEye = normalize(worldPosition - _input.worldPosition);

    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    if (DL_intensity > 0.0f)
    {
        float cos;
        float NdotL = dot(normalize(_input.normal), -DL_direction);
        if (DL_isHalf != 0)
        {
            cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }
        else
        {
            cos = saturate(NdotL);
        }

        float3 halfVector = normalize(-DL_direction + toEye);
        float NDotH = dot(normalize(_input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), shininess);

        diffuse = materialColor.rgb * textureColor.rgb * DL_color.rgb * cos * DL_intensity;
        specular = DL_color.rgb * DL_intensity * specularPow * float3(1.0f, 1.0f, 1.0f);


    }

// Point Lightの計算（PL_intensity > 0 の場合のみ）
    float3 PLDiffuse = float3(0.0f, 0.0f, 0.0f);
    float3 PLSpecular = float3(0.0f, 0.0f, 0.0f);
    if (PL_intensity > 0.0f)
    {
        float3 PLdirection = normalize(_input.worldPosition - PL_position);
        float3 PLHalfVector = normalize(-PLdirection + toEye);
        float PLspecularPow = pow(saturate(dot(normalize(_input.normal), PLHalfVector)), shininess);

        float PL_NdotL = dot(normalize(_input.normal), -PLdirection);
        float PL_cos = saturate(PL_NdotL);
        if (PL_isHalf != 0)
        {
            PL_cos = pow(PL_NdotL * 0.5f + 0.5f, 2.0f);
        }

        float distance = length(PL_position - _input.worldPosition);
        float factor = pow(saturate(-distance / PL_radius + 1.0f), PL_decay);

        PLDiffuse = materialColor.rgb * textureColor.rgb * PL_color.rgb * PL_cos * PL_intensity * factor;
        PLSpecular = PL_color.rgb * PL_intensity * PLspecularPow * float3(1.0f, 1.0f, 1.0f) * factor;

    }

    float3 spotLightcColor = CalculateSpotLighting(_input, toEye, textureColor);

    if (enableLighting != 0)
    {
        output.color.rgb = diffuse + specular + PLDiffuse + PLSpecular + spotLightcColor;
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


float3 CalculateSpotLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    if (SL_intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);


    float3 direction = normalize(_input.worldPosition - SL_position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);

    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (SL_isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }

    float distance = length(SL_position - _input.worldPosition);
    float factor = pow(saturate(-distance / SL_ditance + 1.0f), SL_decay);

    float cosAngle = dot(direction,normalize (SL_direction));
    float falloffFactor = 1.0f;
    if (cosAngle < SL_cosFalloutStart)
    {
        falloffFactor = saturate((cosAngle - SL_cosAngle) / (SL_cosFalloutStart - SL_cosAngle));
    }


    float3 diffuse = materialColor.rgb * _textureColor.rgb * SL_color.rgb * cos * SL_intensity * factor*falloffFactor;
    float3 specular = SL_color.rgb * SL_intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular;

}