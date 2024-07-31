#include "unUV.hlsli" 

cbuffer gMaterial : register(b0)
{
    float4 materialColor;
    int enableLighting;
    float4x4 unTransform;
    float shininess;
};

//平行光源
cbuffer gDirectionalLight : register(b1)
{
    float4 DL_color; //ライトの色
    float3 DL_direction; //ライトの向き
    float DL_intensity; //輝度
    int DL_isHalf;
}

cbuffer Camera : register(b2)
{
    float3 worldPosition;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    
    /*lmbart*/
    
    float cos;
    float NdotL = dot(normalize(_input.normal), -DL_direction);
    if (DL_isHalf != 0)
    { //halfLambertを使うとき
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    else
        cos = saturate(NdotL);
    
   
    float3 toEye = normalize(worldPosition - _input.worldPosition);
    
    float3 halfVector = normalize(-DL_direction + toEye);
    float NDotH = dot(normalize(_input.normal), halfVector);
    //反射強度
    float specularPow = pow(saturate(NDotH), shininess);
    
    //拡散反射
    float3 diffuse = materialColor.rgb * DL_color.rgb * cos * DL_intensity;
    //鏡面反射
    float3 specular = DL_color.rgb * DL_intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

    
    if (enableLighting != 0)
    {
        output.color.rgb = diffuse + specular /*+ PLdiffuse + PLspecular*/;
        output.color.a = materialColor.a;
    }
    else
        output.color = materialColor;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}
