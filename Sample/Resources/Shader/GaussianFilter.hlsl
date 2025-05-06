#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

//
static const int kKernelSize = 5;
static float sigma = 2.0f; // 標準偏差

static const float PI = 3.14159265358979323846f;

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};


float gauss(float _x, float _y, float _sigma)
{
    float exponent = -(_x * _x + _y * _y) * rcp(2.0f * _sigma * _sigma);
    float denominator = 2.0f * PI * _sigma * _sigma;
    return exp(exponent) * rcp(denominator);
}

PixelShaderOutput main(VertexOutput _input)
{
    float width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(width), rcp(height));

    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);

    int kernelSize = kKernelSize;
    // 偶数のとき
    if (kernelSize % 2 == 0)
        kernelSize++;
    if (kernelSize > 25)
        kernelSize = 25;

    const float kKernel = float(1.0f / float(kernelSize * kernelSize));

    float weight = 0.0f;
    const int kMaxKernel = 25;
    float kernelNxN[kMaxKernel][kMaxKernel];
    int x, y;
    for (x = 0; x < kernelSize; ++x)
    {
        for (y = 0; y < kernelSize; ++y)
        {
            float2 index = { x - (kernelSize - 1) / 2, y - (kernelSize - 1) / 2 };

            kernelNxN[x][y] = gauss(index.x, index.y, sigma);
            weight += kernelNxN[x][y];
        }
    }

    for (x = 0; x < kernelSize; ++x)
    {
        for (y = 0; y < kernelSize; ++y)
        {
            float2 index = { x - (kernelSize - 1) / 2, y - (kernelSize - 1) / 2 };

            float2 texcoord = _input.uv + index * uvStepSize;
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;

            output.color.rgb += fetchColor * kernelNxN[x][y];
        }
    }

    output.color.rgb *= rcp(weight);

    output.color.a = 1.0f;
    return output;
}