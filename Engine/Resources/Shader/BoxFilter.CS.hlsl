#include "Resources/Shader/FullScreen.hlsli"

// 入力テクスチャ
Texture2D<float4> gInputTexture : register(t0);
// 出力テクスチャ
RWTexture2D<float4> gOutputTexture : register(u0);


cbuffer KernelSizeBuffer : register(b0)
{
    int kernelSize; // カーネルサイズ

    float3 pad;
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // テクスチャサイズ取得
    uint width, height;
    gInputTexture.GetDimensions(width, height);

    // 範囲外チェック
    if (id.x >= width || id.y >= height)
        return;

    float3 color = float3(0.0f, 0.0f, 0.0f);

    int actualKernelSize = kernelSize;
    // 偶数のとき
    if (actualKernelSize % 2 == 0)
        actualKernelSize++;

    const float kKernel = 1.0f / float(actualKernelSize * actualKernelSize);

    for (int x = 0; x < actualKernelSize; ++x)
    {
        for (int y = 0; y < actualKernelSize; ++y)
        {
            int2 offset = { x - (actualKernelSize - 1) / 2, y - (actualKernelSize - 1) / 2 };
            int2 samplePos = int2(id.xy) + offset;

            // 境界チェック
            samplePos = clamp(samplePos, int2(0, 0), int2(width - 1, height - 1));

            float3 fetchColor = gInputTexture[samplePos].rgb;
            color += fetchColor * kKernel;
        }
    }

    // 出力
    gOutputTexture[id.xy] = float4(color, 1.0f);
}