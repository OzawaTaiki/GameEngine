#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

cbuffer gBuffer : register(b0)
{
    float4x4 gInverseProjectionMatrix;

    float gEdgeThreshold; // エッジの閾値 (0.0~10.0くらい)
    float gEdgeIntensity; // エッジの強度 (0.0~1.0)
    float gEdgeWidth; // エッジの幅 (1.0~3.0くらい)
    int gEnableColorBlending; // カラーブレンディング有効（0:無効, 1:有効）

    float3 gEdgeColor; // エッジの色（RGB）
    float gEdgeColorIntensity; // エッジ色の強度（0.0~1.0）
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float> gDepthTexture : register(t1);
SamplerState gDepthSampler : register(s1);

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },

};

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

static const float kPrewittHorizontal[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
};

static const float kPrewittVertical[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
};

PixelShaderOutput main(VertexOutput _input)
{
    PixelShaderOutput output;

    float width, height;
    gTexture.GetDimensions(width, height);

    output.color = float4(0, 0, 0, gTexture.Sample(gSampler, _input.uv).a);

    float2 uvStepSize = float2(rcp(width), rcp(height));

    float2 difference = float2(0.0f, 0.0f); // 縦横それぞれの結果を格納する変数


    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texcoord = _input.uv + kIndex3x3[x][y] * gEdgeWidth * uvStepSize;

            float ndcDepth = gDepthTexture.Sample(gDepthSampler, texcoord);
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gInverseProjectionMatrix);
            float viewZ = viewSpace.z * rcp(viewSpace.w);

            difference.x += viewZ * kPrewittHorizontal[x][y];
            difference.y += viewZ * kPrewittVertical[x][y];
        }
    }

    float weight = length(difference) * gEdgeIntensity;
    weight = saturate(weight * gEdgeThreshold);
    
    float4 originalColor = gTexture.Sample(gSampler, _input.uv);


    if (gEnableColorBlending)
    {
        // エッジ色とのブレンディング
        float3 edgeColor = lerp(originalColor.rgb, gEdgeColor, weight * gEdgeColorIntensity);
        output.color.rgb = (1.0f - weight) * originalColor.rgb + weight * edgeColor;
    }
    else
    {
        // 従来の暗化処理
        output.color.rgb = (1.0f - weight) * originalColor.rgb;
    }

    return output;
}