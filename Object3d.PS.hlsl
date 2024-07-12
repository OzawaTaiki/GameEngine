#include "Object3d.hlsli" 

cbuffer gMaterial : register(b0)
{
    float4 materialColor;
    int enableLighting;
    float4x4 unTransform;
};

cbuffer gTexVisibility : register(b1)
{
    float isVisible;
};

cbuffer gDirectionalLight : register(b2)
{
    float4 lightColor; //ライトの色
    float3 lightDirection; //ライトの向き
    float intensity; //輝度
    int isHalf;
}
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
    if (isVisible == 1.0f)
    {
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
        textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);
    }
    else
        textureColor = materialColor;
    
    //ライティング
    float cos;
    float NdotL = dot(normalize(_input.normal), -lightDirection);
    if (isHalf != 0)
    { //halfLambertを使うとき
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    else
        cos = saturate(NdotL);
    
    if (enableLighting != 0)
    {
        output.color.rgb = materialColor.rgb * textureColor.rgb * lightColor.rgb * cos * intensity;
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