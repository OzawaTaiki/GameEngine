#include "Object3d.hlsli" 

cbuffer gMaterial : register(b0)
{
    float4 materialColor;
    int enableLighting;
    float4x4 unTransform;
    float shininess;
    float useTexture;
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

////点光源
//cbuffer gPointLight : register(b4)
//{
//    float4 PL_color;
//    float3 PL_position;
//    float PL_intensity;
//}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    //output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 textureColor;
        
    //画像の有無
    if (useTexture > 0.0f)
    {
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
        textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);
    }
    else
        textureColor = materialColor;
    
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
    float3 diffuse = materialColor.rgb * textureColor.rgb * DL_color.rgb * cos * DL_intensity;
    //鏡面反射
    float3 specular = DL_color.rgb * DL_intensity * specularPow * float3(1.0f, 1.0f, 1.0f);


    //float3 PLdirection = normalize(PL_position - _input.worldPosition);
    //float PLNdotL = max(0.0, dot(normalize(_input.normal), PLdirection));
    //float PLcos = pow(PLNdotL * 0.5f + 0.5f, 2.0f);

    //float3 PLhalfVector = normalize(PLdirection + toEye);
    //float PLNDotH = max(0.0, dot(normalize(_input.normal), PLhalfVector));

    //float PLspecularPow = pow(saturate(PLNDotH), shininess);


    //float distance = length(PL_position - _input.worldPosition);
    //float factor = 1.0 / (distance * distance);
    
    //float3 PLdiffuse = materialColor.rgb * textureColor.rgb * PL_color.rgb * PLcos * PL_intensity * factor;
    //float3 PLspecular = PL_color.rgb * PL_intensity * PLspecularPow * float3(1.0f, 1.0f, 1.0f) * factor;

    if (enableLighting != 0)
    {
        output.color.rgb = diffuse + specular /*+ PLdiffuse + PLspecular*/;
        output.color.a = materialColor.a * textureColor.a;
    }
    else
        output.color = materialColor * textureColor;
    
    if (textureColor.a <= 0.5 ||
        textureColor.a == 0.0 ||
        output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}
