#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

//
static const int kKernelSize = 5;

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

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

    const float kKernel = float(1.0f / float(kernelSize * kernelSize));

    for (int x = 0; x < kernelSize; ++x)
    {
        for (int y = 0; y < kernelSize; ++y)
        {
            float2 index = { x - (kernelSize - 1) / 2, y - (kernelSize - 1) / 2 };

            float2 texcoord = _input.uv + index * uvStepSize;
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;

            output.color.rgb += fetchColor * kKernel;
        }
    }

    output.color.a = 1.0f;
    return output;
}