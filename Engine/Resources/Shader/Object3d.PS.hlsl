#include "Resources/Shader/Object3d.hlsli"
//#include "Object3d.hlsli"


cbuffer gMaterial : register(b1)
{
    float4x4 unTransform;

    float4 deffuseColor;

    float shininess;
    int enableLighting;
    int hasTexture;
    float envScale;

    int enableEnviroment;
    float specularStrength;
    float shadingStrength;
    float pad;

    float3 shadeColor;
    float shadeColorPad;
};

cbuffer gColor : register(b2)
{
    float4 materialColor;
}


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float> gShadowMap : register(t1);
SamplerComparisonState gShadowSampler : register(s1);


TextureCube<float> gPointLightShadowMap : register(t2);
SamplerState gPointLightShadowSampler : register(s2);

TextureCube<float4> gEnviromentTexture : register(t3);

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _baseColor);
float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, int _lightIndex, float3 _toEye, float4 _baseColor);
float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _baseColor);

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _baseColor);
float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _baseColor);

float3 CalculateEnViromentColor(VertexShaderOutput _input, float3 _cameraPos);

float ComputeShadow(float4 shadowCoord, float3 worldNormal)
{
    if (DL.castShadow == 0 || shadowCoord.w <= 0.0f)
        return 1.0f;

    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = -shadowCoord.y * 0.5 + 0.5;

    // 範囲外チェック
    if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0 ||
        shadowCoord.y < 0.0 || shadowCoord.y > 1.0 ||
        shadowCoord.z < 0.0 || shadowCoord.z > 1.0)
        return 1.0f;

    // 深度レンジは約200mなので、0.001でも約20cmに相当する。
    // 以前はreceiverOffsetとbiasが二重に掛かり、背の低い物体の影を消していた。
    float3 lightDir = normalize(-DL.direction);
    float NdotL = max(dot(normalize(worldNormal), lightDir), 0.0);
    float bias = 0.00005f + 0.0002f * (1.0f - NdotL);

    uint shadowWidth;
    uint shadowHeight;
    gShadowMap.GetDimensions(shadowWidth, shadowHeight);
    float2 texelSize = 1.0f / float2(shadowWidth, shadowHeight);

    // 3x3 PCF。比較結果1が照明、0が遮蔽。
    float visibility = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            visibility += gShadowMap.SampleCmpLevelZero(
                gShadowSampler,
                shadowCoord.xy + float2(x, y) * texelSize,
                shadowCoord.z - bias);
        }
    }
    visibility /= 9.0f;

    return lerp(DL.shadowFactor, 1.0f, visibility);
}


float ComputePointLightShadow(int lightIndex, float3 worldPos, float3 _normal, PointLight _PL)
{
    // 安全性のチェック
    if (lightIndex < 0 || lightIndex >= MAX_POINT_LIGHT || !_PL.castShadow)
        return 1.0f;

    // ライト位置から現在のワールド座標へのベクトル計算
    float3 lightToWorldVec = worldPos - _PL.position;

    // シャドウ計算のロジック
    float currentDepth = length(lightToWorldVec) / _PL.radius;

    // 対応するライトのシャドウマップをサンプリング
    float closestDepth = gPointLightShadowMap.Sample(
        gPointLightShadowSampler,
        lightToWorldVec
    ).r;

    float3 lightDir = normalize(_PL.position - worldPos);
    float NdotL = max(dot(_normal, lightDir), 0.0);
    float bias = 0.005 + 0.015 * (1.0 - NdotL); // 角度に応じて調整
    float shadow = currentDepth > closestDepth + bias ? _PL.shadowFactor : 1.0;


    return shadow;
}



PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 baseColor = deffuseColor * materialColor;

    if (hasTexture != 0)
    {
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
        baseColor *= gTexture.Sample(gSampler, transformedUV.xy);
    }

    float3 toEye = normalize(worldPosition - _input.worldPosition);

    if (enableLighting != 0)
    {
        // シャドウファクターを適用したライティング
        float3 directionalLight = CalculateDirectionalLighting(_input, toEye, baseColor) * ComputeShadow(_input.shadowPos, _input.normal);
        float3 pointLight = CalculateLightingWithMultiplePointLights(_input, toEye, baseColor);
        float3 spotLightColor = CalculateLightingWithMultipleSpotLights(_input, toEye, baseColor);

        float3 envColor = float3(0,0,0);
        if (enableEnviroment != 0)
            envColor = CalculateEnViromentColor(_input, worldPosition) * envScale;

        output.color.rgb = directionalLight + pointLight + spotLightColor + envColor;
        output.color.a = baseColor.a;
    }
    else
        output.color = baseColor;

    if (baseColor.a == 0.0)
    {
        discard;
    }

    return output;
}

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _baseColor)
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
    float3 shade = lerp(shadeColor, float3(1.0f, 1.0f, 1.0f), cos);
    shade = lerp(float3(1.0f, 1.0f, 1.0f), shade, shadingStrength);
    float3 diffuse = _baseColor.rgb * DL.color.rgb * shade * DL.intensity;
    float3 specular = DL.color.rgb * DL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

    return diffuse + specular * specularStrength;
}

float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, int _lightIndex, float3 _toEye, float4 _baseColor)
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
    float3 shade = lerp(shadeColor, float3(1.0f, 1.0f, 1.0f), cos);
    shade = lerp(float3(1.0f, 1.0f, 1.0f), shade, shadingStrength);
    float distance = length(_PL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _PL.radius + 1.0f), _PL.decay);

    float shadowFactor = ComputePointLightShadow(_lightIndex, _input.worldPosition, _input.normal,_PL);

    float3 diffuse = _baseColor.rgb * _PL.color.rgb * shade * _PL.intensity * factor * shadowFactor;
    float3 specular = _PL.color.rgb * _PL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * shadowFactor;

    return diffuse + specular * specularStrength;

}

float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _baseColor)
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
    float3 shade = lerp(shadeColor, float3(1.0f, 1.0f, 1.0f), cos);
    shade = lerp(float3(1.0f, 1.0f, 1.0f), shade, shadingStrength);

    float distance = length(_SL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _SL.distance + 1.0f), _SL.decay);

    float cosAngle = dot(direction, normalize(_SL.direction));
    float falloffFactor = 1.0f;
    if (cosAngle < _SL.cosFalloutStart)
    {
        falloffFactor = saturate((cosAngle - _SL.cosAngle) / (_SL.cosFalloutStart - _SL.cosAngle));
    }


    float3 diffuse = _baseColor.rgb * _SL.color.rgb * shade * _SL.intensity * factor * falloffFactor;
    float3 specular = _SL.color.rgb * _SL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular * specularStrength;

}

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _baseColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numPointLight; i++)
    {
        lighting += CalculatePointLighting(_input, PL[i], i, _toEye, _baseColor);
    }
    return lighting;
}

float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _baseColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numSpotLight; i++)
    {
        lighting += CalculateSpotLighting(_input, SL[i], _toEye, _baseColor);
    }
    return lighting;
}

float3 CalculateEnViromentColor(VertexShaderOutput _input, float3 _cameraPos)
{

    float3 cameraToPosition = normalize(_input.worldPosition - _cameraPos);
    float3 reflectVector = reflect(cameraToPosition, normalize(_input.normal));
    float4 envColor = gEnviromentTexture.Sample(gSampler, reflectVector);

    return envColor.rgb;
}
