#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

cbuffer gCameraMat : register(b0)
{
    float4x4 gInverseProjectionMatrix;
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
    float2 uvStepSize = float2(rcp(width), rcp(height));

    float2 difference = float2(0.0f, 0.0f); // 縦横それぞれの結果を格納する変数

    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texcoord = _input.uv + kIndex3x3[x][y] * uvStepSize;

            float ndcDepth = gDepthTexture.Sample(gDepthSampler, texcoord);
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gInverseProjectionMatrix);// TODO
            float viewZ = viewSpace.z * rcp(viewSpace.w);

            difference.x += viewZ * kPrewittHorizontal[x][y];
            difference.y += viewZ * kPrewittVertical[x][y];
        }
    }

    float weight = length(difference);
    weight = saturate(weight); // 0~1に制限

    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, _input.uv).rgb;
    output.color.a = 1.0f;

    return output;
}