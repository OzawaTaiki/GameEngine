#include  "Resources/Shader/FullScreen.hlsli"


cbuffer ConstantBuffer : register(b0)
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

Texture2D<float> gDepthTexture : register(t1);

RWTexture2D<float4> gOutputTexture : register(u0);


static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },

};

static const float kSobelHorizontal[3][3] =
{
    { -1.0f / 8.0f, 0.0f, 1.0f / 8.0f },
    { -2.0f / 8.0f, 0.0f, 2.0f / 8.0f },
    { -1.0f / 8.0f, 0.0f, 1.0f / 8.0f }
};

static const float kSobelVertical[3][3] =
{
    { -1.0f /8.0f, -2.0f / 8.0f, -1.0f / 8.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f }
};

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{

    uint width, height;
    gTexture.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;

    float2 difference = float2(0.0f, 0.0f); // 縦横それぞれの結果を格納する変数

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            int2 texcoord = int2(id.xy) + int2(kIndex3x3[x][y] * gEdgeWidth);
            texcoord = clamp(texcoord, int2(0, 0), int2(width - 1, height - 1));

            float ndcDepth = gDepthTexture[texcoord];
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gInverseProjectionMatrix); // TODO
            float viewZ = viewSpace.z * rcp(viewSpace.w);

            difference.x += viewZ * kSobelHorizontal[x][y];
            difference.y += viewZ * kSobelVertical[x][y];
        }
    }

    float weight = length(difference) * gEdgeIntensity;
    weight = saturate(weight * gEdgeThreshold);

    float4 originalColor = gTexture[id.xy];

    float4 output;

    if (gEnableColorBlending)
    {
        // エッジ色とのブレンディング
        float3 edgeColor = lerp(originalColor.rgb, gEdgeColor, weight * gEdgeColorIntensity);
        output.rgb = (1.0f - weight) * originalColor.rgb + weight * edgeColor;
    }
    else
    {
        // 従来の暗化処理
        output.rgb = (1.0f - weight) * originalColor.rgb;
    }

    output.a = 1.0f;


    gOutputTexture[id.xy] = output;
}