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
    float3 pad;
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

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, int _lightIndex, float3 _toEye, float4 _textureColor);
float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _textureColor);

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);

float3 CalculateEnViromentColor(VertexShaderOutput _input, float3 _cameraPos);

float ComputeShadow(float4 shadowCoord, float3 worldNormal)
{
    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = -shadowCoord.y * 0.5 + 0.5;

    // 範囲外チェック
    if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0 ||
        shadowCoord.y < 0.0 || shadowCoord.y > 1.0 ||
        shadowCoord.z < 0.0 || shadowCoord.z > 1.0)
        return 1.0f;

    float receiverOffset = 0.001f; // 調整可能
    float currentDepth = shadowCoord.z - receiverOffset;

    float closestDepth = gShadowMap.Sample(gSampler, shadowCoord.xy).r;

    // 法線ベースの動的bias（Shadow Acne対策）
    float3 lightDir = normalize(-DL.direction);
    float NdotL = max(dot(normalize(worldNormal), lightDir), 0.0);
    float bias = 0.001 + 0.005 * (1.0 - NdotL); // 角度に応じて調整

    float shadow = (currentDepth > closestDepth + bias) ? DL.shadowFactor : 1.0f;
    return shadow;
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
    float4 textureColor = deffuseColor * materialColor;

    if (hasTexture != 0)
    {
        float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
        textureColor *= gTexture.Sample(gSampler, transformedUV.xy);
    }

    float3 toEye = normalize(worldPosition - _input.worldPosition);

    if (enableLighting != 0)
    {
        // シャドウファクターを適用したライティング
        float3 directionalLight = CalculateDirectionalLighting(_input, toEye, textureColor) * ComputeShadow(_input.shadowPos,_input.normal);
        float3 pointLight = CalculateLightingWithMultiplePointLights(_input, toEye, textureColor);
        float3 spotLightcColor = CalculateLightingWithMultipleSpotLights(_input, toEye, textureColor);

        float3 envColor = float3(0,0,0);
        if (enableEnviroment != 0)
            envColor = CalculateEnViromentColor(_input, worldPosition) * envScale;

        output.color.rgb = directionalLight + pointLight + spotLightcColor + envColor;
        output.color.a = deffuseColor.a * textureColor.a;
    }
    else
        output.color = deffuseColor * textureColor;

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
    float3 diffuse = deffuseColor.rgb * _textureColor.rgb * DL.color.rgb * cos * DL.intensity;
    float3 specular = DL.color.rgb * DL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

    return diffuse + specular;
}

float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, int _lightIndex, float3 _toEye, float4 _textureColor)
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

    float shadowFactor = ComputePointLightShadow(_lightIndex, _input.worldPosition, _input.normal,_PL);

    float3 diffuse = deffuseColor.rgb * _textureColor.rgb * _PL.color.rgb * cos * _PL.intensity * factor * shadowFactor;
    float3 specular = _PL.color.rgb * _PL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * shadowFactor;

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


    float3 diffuse = deffuseColor.rgb * _textureColor.rgb * _SL.color.rgb * cos * _SL.intensity * factor * falloffFactor;
    float3 specular = _SL.color.rgb * _SL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular;

}

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numPointLight; i++)
    {
        lighting += CalculatePointLighting(_input, PL[i], i, _toEye, _textureColor);
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

float3 CalculateEnViromentColor(VertexShaderOutput _input, float3 _cameraPos)
{

    float3 cameraToPosition = normalize(_input.worldPosition - _cameraPos);
    float3 reflectVector = reflect(cameraToPosition, normalize(_input.normal));
    float4 envColor = gEnviromentTexture.Sample(gSampler, reflectVector);

    return envColor.rgb;
}
